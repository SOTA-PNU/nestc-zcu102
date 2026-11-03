#!/usr/bin/env python3
"""번들 추론 함수의 문장마다 시간을 재는 코드를 끼워 넣는다 (복사본 전용).

    python3 nt_instrument.py <bundle.cpp> <태그>

문장 앞에 _nt_mark(i) 를 넣어 직전 문장이 걸린 시간을 그 문장의 함수 이름
(quantize, convolution_wo_tr, VTABufferAlloc ...) 칸에 더한다. 스레드가 끝날 때
thread_local 소멸자가 한 줄로 평균을 stderr 에 찍는다.
"""
import re
import sys

path, tag = sys.argv[1], sys.argv[2]
src = open(path).read()
m = re.search(r'^int (\w+)\(uint8_t \*constantWeight, uint8_t \*mutableWeight, '
              r'uint8_t \*activations\)\{\n', src, re.M)
if not m:
    sys.exit("추론 함수를 찾지 못했다: " + path)
start = m.end()
end = src.find('\n}\n', start)
body = src[start:end].split('\n')

names = []
def cat(line):
    s = line.strip()
    c = re.search(r'(?<![\w*])([A-Za-z_]\w*)\(', s)
    # 캐스트 "(int8_t *)" 다음의 이름이 호출 대상이다
    calls = [x for x in re.findall(r'([A-Za-z_]\w*)\(', s) if x not in ('int8_t', 'int32_t', 'uint8_t')]
    n = calls[0] if calls else 'other'
    if n not in names:
        names.append(n)
    return names.index(n)

out = ['  _nt_begin();']
for line in body:
    s = line.strip()
    if not s or s.startswith('//'):
        out.append(line)
        continue
    if s.startswith('return'):
        out.append('  _nt_end();')
        out.append(line)
        continue
    out.append('  _nt_mark(%d);' % cat(line))
    out.append(line)
if not any(l.strip().startswith('return') for l in body):
    out.append('  _nt_end();')

K = len(names)
hdr = r'''
// ---- nt_instrument: 문장별 시간 (측정용 복사본) ----
#include <time.h>
#include <stdio.h>
#include <unistd.h>
namespace {
const char *_nt_names[%(K)d] = {%(names)s};
inline double _nt_now() { timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e3 + t.tv_nsec * 1e-6; }
struct _NtStat {
  double acc[%(K)d]; int cur; double last, t0, first, lastEnd, inf; int count;
  _NtStat() { for (int i = 0; i < %(K)d; i++) acc[i] = 0; cur = -1; last = t0 = 0; first = -1; lastEnd = 0; inf = 0; count = 0; }
  ~_NtStat() {
    if (!count) return;
    char b[8192]; int o = snprintf(b, sizeof b, "NT %(tag)s n=%%d first=%%.1f lastEnd=%%.1f inf=%%.2f", count, first, lastEnd, inf / count);
    for (int i = 0; i < %(K)d; i++) if (acc[i] > 0 && o < 8000) o += snprintf(b + o, sizeof b - o, " %%s=%%.3f", _nt_names[i], acc[i] / count);
    b[o++] = '\n'; ssize_t w = write(2, b, o); (void)w;
  }
};
thread_local _NtStat _nt;
inline void _nt_begin() { double n = _nt_now(); if (_nt.first < 0) _nt.first = n; _nt.t0 = _nt.last = n; _nt.cur = -1; }
inline void _nt_mark(int i) { double n = _nt_now(); if (_nt.cur >= 0) _nt.acc[_nt.cur] += n - _nt.last; _nt.cur = i; _nt.last = n; }
inline void _nt_end() { _nt_mark(-1); double n = _nt.last; _nt.inf += n - _nt.t0; _nt.lastEnd = n; _nt.count++; }
}
// ---- nt_instrument 끝 ----
''' % {'K': K, 'names': ', '.join('"%s"' % n for n in names), 'tag': tag}

# _nt_mark(-1) 은 cur 를 -1 로 만든다 — 마지막 문장의 시간을 더한 뒤 멈춘다
func_pos = m.start()
new = src[:func_pos] + hdr + src[func_pos:start] + '\n'.join(out) + src[end:]
open(path, 'w').write(new)
print("%s: 문장 %d개, 분류 %d개 (%s)" % (path, sum(1 for l in out if '_nt_mark(' in l), K, ' '.join(names)))
