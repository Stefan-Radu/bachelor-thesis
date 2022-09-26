import matplotlib.pyplot as plt
import numpy as np

points = np.array([int(x) for x in input().split()])
avg = np.mean(points)
points[points > 400] = avg

y = np.array(range(256))

plt.plot(y, points);
plt.show()
