import json
import matplotlib.pyplot as plt
import numpy as np

# Load the metrics from file
with open("data.txt", "r") as file:
    lines = file.readlines()

json_data = "".join(lines[1:])
data = json.loads(json_data)

# Plotting function
def plot_line(y, title, ylabel, xlabel="Time", filename="plot.png", color="blue", label=None):
    plt.figure(figsize=(10, 4))
    x = range(len(y))
    plt.plot(x, y, marker='o', markersize=6, linewidth=2.5, linestyle='-', color=color, label=label or title)
    plt.title(title, fontsize=16)
    plt.xlabel(xlabel, fontsize=14)
    plt.ylabel(ylabel, fontsize=14)
    plt.xticks(fontsize=12)
    plt.yticks(fontsize=12)
    plt.grid(True)
    plt.legend(fontsize=12)
    plt.tight_layout()
    plt.savefig(filename)
    plt.show()


# 1. DEX Fees A
plot_line(
    [v / 1e18 for v in data["dexFeesA"]],
    title="DEX Fees from Token A Over Time",
    ylabel="Fees",
    xlabel="Time",
    filename="dex_fees_A.png",
    color="purple"
)


# 2. DEX Fees B
plot_line(
    [v / 1e18 for v in data["dexFeesB"]],
    title="DEX Fees from Token B Over Time",
    ylabel="Fees",
    xlabel="Time",
    filename="dex_fees_B.png",
    color="purple"
)

# 3. DEX Fees
plot_line(
    [v / 1e18 for v in data["dexFees"]],
    title="Total DEX Fees Over Time",
    ylabel="Fees",
    xlabel="Time",
    filename="dex_fees.png",
    color="purple"
)

# 4. TVL
plot_line(
    data["tvl"],
    title="Total Value Locked (TVL)",
    ylabel="TVL (in terms of Token A)",
    xlabel="Time",
    filename="tvl.png",
    color="green"
)

# 5. Spot Price
plot_line(
    data["spotPrices"][1:],
    title="Spot Price (Token A / Token B)",
    ylabel="Spot Price",
    xlabel="Time",
    filename="spot_price.png",
    color="orange"
)

# 6. Slippage
plot_line(
    data["slippage"][1:],
    title="Slippage Over Time",
    ylabel="Slippage (%)",
    xlabel="Time",
    filename="slippage.png",
    color="red"
)

# 7. Reserve Ratios
plot_line(
    data["reserveRatios"][1:],
    title="Reserve Ratio (A/B)",
    ylabel="Ratio",
    xlabel="Time",
    filename="reserve_ratio.png",
    color="blue"
)

# 8. LP Token Distribution Ratio
lp_array = np.array(data["lpTokenDistributionRatio"])
if lp_array.ndim == 2:
    plt.figure(figsize=(10, 4))
    for i in range(lp_array.shape[1]):
        plt.plot(range(len(lp_array)), lp_array[:, i], marker='o', markersize=3, label=f"LP {i+1}")
    plt.title("LP Token Distribution Fraction Over Time")
    plt.xlabel("Time")
    plt.ylabel("LP Share")
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.savefig("lp_distribution_ratio.png")
    plt.show()

# 9. LP Token Distribution
lp_array = np.array(data["lpTokenDistribution"])/1e18
if lp_array.ndim == 2:
    plt.figure(figsize=(10, 4))
    for i in range(lp_array.shape[1]):
        plt.plot(range(len(lp_array)), lp_array[:, i], marker='o', markersize=3, label=f"LP {i+1}")
    plt.title("LP Token Distribution Over Time")
    plt.xlabel("Time")
    plt.ylabel("LP tokens holdings")
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.savefig("lp_distribution.png")
    plt.show()

# 10. Swap Volume A
plot_line(
    [v / 1e18 for v in data["swapVolumeA"]],
    title="Swap Volume (Token A)",
    ylabel="Token A Swapped",
    xlabel="Time",
    filename="swap_volume_a.png",
    color="teal"
)

# 11. Swap Volume B
plot_line(
    [v / 1e18 for v in data["swapVolumeB"]],
    title="Swap Volume (Token B)",
    ylabel="Token B Swapped",
    xlabel="Time",
    filename="swap_volume_b.png",
    color="chocolate"
)

# 12. Slippage vs Trade Lot Fraction Plot (Constant Product AMM)
f = np.linspace(0.00, 0.10, 10)[1:] 
slippage_theoretical = -(f / (1 + f)) * 100

plt.figure(figsize=(8, 5))
plt.plot(f, slippage_theoretical, label='Slippage vs Trade Lot Fraction', color='blue')
plt.xlabel('Trade Lot Fraction (Δx / x)', fontsize=12)
plt.ylabel('Slippage (%)', fontsize=12)
plt.title('Slippage vs. Trade Lot Fraction for Constant Product AMM', fontsize=14)
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("slippage_vs_trade_lot_fraction.png")
plt.show()