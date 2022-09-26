import fileinput

lines = []
total = 0
for line in fileinput.input():
  if len(line) < 2:
    break
  a, b = (int(x) for x in line.split())
  lines.append((a, b))
  total += b

l = len(lines)

result = 0
for a, b in lines:
  result += a * b / total

print(round(result))
