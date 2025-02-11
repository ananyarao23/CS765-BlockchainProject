import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.lines as mlines

# Load the data
df = pd.read_csv("simulation_results.csv")

# Convert categorical values for plotting
colors = df["CPU"].map({"Low": "blue", "High": "red"})  # Red = High CPU, Blue = Low CPU
markers = df["Speed"].map({"Fast": "o", "Slow": "x"})   # 'o' = Fast, 'x' = Slow

# Create a figure with 3 subplots
fig, axs = plt.subplots(1, 3, figsize=(18, 5))

# 1. Fast vs. Slow (Ignore CPU power)
axs[0].scatter(df[df["Speed"] == "Fast"].index, df[df["Speed"] == "Fast"]["Ratio"], color="green", marker="o", label="Fast Nodes")
axs[0].scatter(df[df["Speed"] == "Slow"].index, df[df["Speed"] == "Slow"]["Ratio"], color="purple", marker="x", label="Slow Nodes")
axs[0].set_title("Fast vs Slow Nodes")
axs[0].set_xlabel("Node Index")
axs[0].set_ylabel("Block Acceptance Ratio")
axs[0].legend()

# 2. High vs. Low CPU (Ignore Speed)
axs[1].scatter(df[df["CPU"] == "High"].index, df[df["CPU"] == "High"]["Ratio"], color="red", marker="o", label="High CPU")
axs[1].scatter(df[df["CPU"] == "Low"].index, df[df["CPU"] == "Low"]["Ratio"], color="blue", marker="x", label="Low CPU")
axs[1].set_title("High vs Low CPU Nodes")
axs[1].set_xlabel("Node Index")
axs[1].set_ylabel("Block Acceptance Ratio")
axs[1].legend()

# 3. Combined Plot (With Legend)
for i, row in df.iterrows():
    axs[2].scatter(i, row["Ratio"], color=colors[i], marker=markers[i])

axs[2].set_title("Combined: Speed & CPU Impact")
axs[2].set_xlabel("Node Index")
axs[2].set_ylabel("Block Acceptance Ratio")

# Add legend manually to the combined plot
legend_handles = [
    mlines.Line2D([], [], color='red', marker='o', linestyle='None', markersize=8, label='Fast, High CPU'),
    mlines.Line2D([], [], color='red', marker='x', linestyle='None', markersize=8, label='Slow, High CPU'),
    mlines.Line2D([], [], color='blue', marker='o', linestyle='None', markersize=8, label='Fast, Low CPU'),
    mlines.Line2D([], [], color='blue', marker='x', linestyle='None', markersize=8, label='Slow, Low CPU')
]
axs[2].legend(handles=legend_handles)

# Adjust layout and save
plt.tight_layout()
plt.savefig("ratio_analysis.png")
plt.show()
