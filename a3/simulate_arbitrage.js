async function main() {
    const accounts = await web3.eth.getAccounts();

    const arbitrageur = accounts[0];

    // replace with addresses of deployed contracts
    const tokenAddressA = "0xaC7C1040FB57acb60A9F6198a639B39D796E836d";
    const tokenAddressB = "0x6F7d90bcB4c2f393Ef494e2d5c8b15364432734c";
    const arbitrageAddress = "0xF24DE17D347D4c0Fd2A90E0a63a71381e922CCeC";
    const dexAddress1 = "0x572b8DFd97a7AE6097B9522aFEF6067035403833";
    const dexAddress2 = "0x6003516C84C127A07f5Be1Ece4c357AD23a175C9";

    // deploying all the contracts
    const metadataA = JSON.parse(await remix.call('fileManager', 'getFile', 'a3/contracts/artifacts/TokenA.json'));
    if (!metadataA) {
        throw new Error("Could not find TokenA.json artifact. Please compile the contract first.");
    }
    const tokenAbiA = metadataA.abi;
    const tokenA = new web3.eth.Contract(tokenAbiA, tokenAddressA);

    const metadataB = JSON.parse(await remix.call('fileManager', 'getFile', 'a3/contracts/artifacts/TokenB.json'));
    if (!metadataB) {
        throw new Error("Could not find TokenB.json artifact. Please compile the contract first.");
    }
    const tokenAbiB = metadataB.abi;
    const tokenB = new web3.eth.Contract(tokenAbiB, tokenAddressB);

    const metadataDEX = JSON.parse(await remix.call('fileManager', 'getFile', 'a3/contracts/artifacts/DEX.json'));
    if (!metadataDEX) {
        throw new Error("Could not find DEX.json artifact. Please compile the contract first.");
    }
    const tokenAbiDEX = metadataDEX.abi;
    const dex1 = new web3.eth.Contract(tokenAbiDEX, dexAddress1);
    const dex2 = new web3.eth.Contract(tokenAbiDEX, dexAddress2);

    const metadataLP = JSON.parse(await remix.call('fileManager', 'getFile', 'a3/contracts/artifacts/LPToken.json'));
    if (!metadataLP) {
        throw new Error("Could not find LPToken.json artifact. Please compile the LPToken contract first.");
    }
    const lpTokenAbi = metadataLP.abi;
    const lpTokenAddress1 = await dex1.methods.lpToken().call();
    const lpToken1 = new web3.eth.Contract(lpTokenAbi, lpTokenAddress1);
    const lpTokenAddress2 = await dex2.methods.lpToken().call();
    const lpToken2 = new web3.eth.Contract(lpTokenAbi, lpTokenAddress2);

    const metadataArb = JSON.parse(await remix.call('fileManager', 'getFile', 'a3/contracts/artifacts/arbitrage.json'));
    if (!metadataArb) {
        throw new Error("Could not find arbitrage.json artifact. Please compile the contract first.");
    }
    const tokenAbiArb = metadataArb.abi;
    const arb = new web3.eth.Contract(tokenAbiArb, arbitrageAddress);

    console.log("Tokens deployed");

    const amount1 = web3.utils.toWei("1", "ether");
    const amount100 = web3.utils.toWei("100", "ether");
    const amount300 = web3.utils.toWei("300", "ether");


    // setup failed arbitrage case
    try {
        await tokenA.methods.approve(dexAddress1, amount100).send({ from: arbitrageur });
        await tokenB.methods.approve(dexAddress1, amount100).send({ from: arbitrageur });
    } catch (e) { console.log(e); }

    try {
        await dex1.methods.addLiquidity(web3.utils.toWei("100", "ether"), web3.utils.toWei("100", "ether")).send({ from: arbitrageur });
    } catch (e) { console.log(e); }

    try {
        await tokenA.methods.approve(dexAddress2, amount100).send({ from: arbitrageur });
        await tokenB.methods.approve(dexAddress2, amount100).send({ from: arbitrageur });
        await dex2.methods.addLiquidity(web3.utils.toWei("100", "ether"), web3.utils.toWei("100", "ether")).send({ from: arbitrageur });
    }
    catch (e) { console.log(e); }

    console.log("Failed arbitrage case setup");


    // give arbitrageur 1 eth and approving contract to spend it
    await tokenA.methods.transfer(arbitrageAddress, web3.utils.toWei("1", "ether")).send({ from: arbitrageur });
    await tokenA.methods.approve(arbitrageAddress, web3.utils.toWei("1", "ether")).send({ from: arbitrageur });

    try {
        const amountIn = web3.utils.toWei("1", "ether").toString();

        await arb.methods.arbitrage(
            dexAddress1,
            dexAddress2,
            tokenAddressA,
            tokenAddressB,
            amountIn
        ).send({ from: arbitrageur });

        console.log("Arbitrage wrongly executed!");
    } catch (e) {
        console.error("Correctly failed non-profitable arbitrage:");
    }

    // return;

    // setup successful arbitrage case
    try {
        await tokenA.methods.approve(dexAddress2, amount1).send({ from: arbitrageur });
        await tokenB.methods.approve(dexAddress2, amount300).send({ from: arbitrageur });
        await dex2.methods.addLiquidity(web3.utils.toWei("1", "ether"), web3.utils.toWei("300", "ether")).send({ from: arbitrageur });
    }
    catch (e) { console.log("!", e); }

    console.log("Successful arbitrage case setup.");

    try {
        const amountIn = web3.utils.toWei("1", "ether").toString();

        await arb.methods.arbitrage(
            dexAddress1,
            dexAddress2,
            tokenAddressA,
            tokenAddressB,
            amountIn
        ).send({ from: arbitrageur });

        console.log("Successful arbitrage execution!");
    } catch (e) {
        console.error("Failed arbitrage execution:");
    }

    console.log("Finished simulation");
}

main();
