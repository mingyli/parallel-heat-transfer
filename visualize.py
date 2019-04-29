import argparse
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import matplotlib.cm as colormap


def main():
    parser = argparse.ArgumentParser(
        description="Visualize time series data of heat transfer"
    )
    parser.add_argument("file")
    args = parser.parse_args()
    data = np.genfromtxt(
        args.file,
        delimiter=",",
        skip_header=1,
        dtype=[
            ("timestamp", float, 1),
            ("position", float, 2),
            ("temperature", float, 1),
        ],
    )

    timestamps = np.sort(np.unique(data["timestamp"]))

    x = data["position"][:, 0]
    y = data["position"][:, 1]
    fig = plt.figure()
    ax = fig.add_subplot(111, xlim=(min(x), max(x)), ylim=(min(y), max(y)))
    norm = matplotlib.colors.Normalize(vmin=min(data["temperature"]), vmax=max(data["temperature"]))
    points = ax.scatter([], [], c=[], cmap="jet", norm=norm)

    def animate(time):
        segment = data[time == data["timestamp"]]
        temperature = segment["temperature"]
        points.set_color(colormap.jet(norm(temperature)))
        points.set_offsets(segment["position"])
        return (points,)

    fig.colorbar(points, ax=ax)
    anim = animation.FuncAnimation(fig, animate, frames=timestamps)
    plt.show()


def generate():
    x = np.random.randn(1000)
    y = np.random.randn(1000)

    points = None
    for timestep in np.linspace(0, 10):
        t = np.ones(1000) * timestep
        temperature = np.sin((x + t) ** 2 + y ** 2) / ((x + t) ** 2 + y ** 2)
        g = np.stack((t, x, y, temperature), axis=-1)
        if points is None:
            points = g
        else:
            points = np.append(points, g, axis=0)

    np.savetxt(
        "./data/sample.csv",
        points,
        delimiter=",",
        header="Timestamp,X,Y,Temperature",
    )


if __name__ == "__main__":
    main()
