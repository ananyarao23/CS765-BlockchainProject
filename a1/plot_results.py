import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.lines as mlines

# Load the data
df = pd.read_csv("simulation_results.csv")

# Convert categorical values for plotting
colors = df["CPU"].map({"Low": "blue", "High": "red"})  # Red = High CPU, Blue = Low CPU
markers = df["Speed"].map({"Fast": "o", "Slow": "x"})   # 'o' = Fast, 'x' = Slow

# 1. Fast vs Slow Nodes
plt.figure(figsize=(8, 5))
plt.scatter(df[df["Speed"] == "Fast"].index, df[df["Speed"] == "Fast"]["Ratio"], color="green", marker="o", label="Fast Nodes")
plt.scatter(df[df["Speed"] == "Slow"].index, df[df["Speed"] == "Slow"]["Ratio"], color="purple", marker="x", label="Slow Nodes")
plt.xlabel("Node Index")
plt.ylabel("Block Acceptance Ratio")
plt.title("Fast vs Slow Nodes")
plt.legend()
plt.savefig("ratio_fast_vs_slow.png")
plt.show()

# 2. High vs Low CPU Nodes
plt.figure(figsize=(8, 5))
plt.scatter(df[df["CPU"] == "High"].index, df[df["CPU"] == "High"]["Ratio"], color="red", marker="o", label="High CPU")
plt.scatter(df[df["CPU"] == "Low"].index, df[df["CPU"] == "Low"]["Ratio"], color="blue", marker="x", label="Low CPU")
plt.xlabel("Node Index")
plt.ylabel("Block Acceptance Ratio")
plt.title("High vs Low CPU Nodes")
plt.legend()
plt.savefig("ratio_high_vs_low_cpu.png")
plt.show()

# 3. Combined Plot (Speed & CPU Impact)
plt.figure(figsize=(8, 5))
for i, row in df.iterrows():
    plt.scatter(i, row["Ratio"], color=colors[i], marker=markers[i])

plt.xlabel("Node Index")
plt.ylabel("Block Acceptance Ratio")
plt.title("Combined: Speed & CPU Impact")

# Add legend manually
legend_handles = [
    mlines.Line2D([], [], color='red', marker='o', linestyle='None', markersize=8, label='Fast, High CPU'),
    mlines.Line2D([], [], color='red', marker='x', linestyle='None', markersize=8, label='Slow, High CPU'),
    mlines.Line2D([], [], color='blue', marker='o', linestyle='None', markersize=8, label='Fast, Low CPU'),
    mlines.Line2D([], [], color='blue', marker='x', linestyle='None', markersize=8, label='Slow, Low CPU')
]
plt.legend(handles=legend_handles)
plt.savefig("ratio_combined.png")
plt.show()
