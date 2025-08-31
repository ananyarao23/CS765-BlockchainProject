(async () => {
    console.clear();
    console.log("Simulation starting in Remix...");

    const accounts = await web3.eth.getAccounts();
    
    // replace with addresses of deployed contracts
    const tokenAddressA = "0x56a2777e796eF23399e9E1d791E1A0410a75E31b";
    const tokenAddressB = "0x8059B0AE35c113137694Ba15b2C3585aE77Bb8E9";
    const addressDEX = "0x86cA07C6D491Ad7A535c26c5e35442f3e26e8497";

    const LPs = accounts.slice(0, 5); //5 LPs
    const traders = accounts.slice(5, 13); // traders
    const BN = web3.utils.BN;

    const N = 75; // no of transactions

    const metrics = {
        dexFeesA: [0],
        dexFeesB: [0],
        dexFees: [0],
        lpTokenDistribution: [[0, 0, 0, 0, 0]],
        lpTokenDistributionRatio: [[0, 0, 0, 0, 0]],
        reserveRatios: [0],
        slippage: [0],
        spotPrices: [0],
        swapVolumeA: [0],
        swapVolumeB: [0],
        tvl: [0]
    };

    function pickRandom(arr) {
        return arr[Math.floor(Math.random() * arr.length)];
    }


    function getRandomAmount(maxBN) {
        const rand = Math.floor(Math.random() * (1e6 - 1)) + 1; 
        return maxBN.muln(rand).divn(1e6);
    }


    function getRandomSwapAmount(userTokBN, reserveTokBN) {
        const tenPercentReserve = reserveTokBN.divn(10); 
        const max = BN.min(userTokBN, tenPercentReserve); 
        //tokens deposited in a swap chosen from a uniform random distribution 
        //between 0 and min(tokens held by user, 10% of reserves of that token)

        if (max.isZero()) {
            return new BN(1); 
        }

        return getRandomAmount(max);
    }


    const metadataA = JSON.parse(await remix.call('fileManager', 'getFile', 'contracts/artifacts/TokenA.json'));

    if (!metadataA) {
        throw new Error("Could not find TokenA.json artifact. Please compile the contract first.");
    }

    const tokenAbiA = metadataA.abi;

    const tokenA = new web3.eth.Contract(tokenAbiA, tokenAddressA);



    const metadataB = JSON.parse(await remix.call('fileManager', 'getFile', 'contracts/artifacts/TokenB.json'));
    if (!metadataB) {
        throw new Error("Could not find TokenB.json artifact. Please compile the contract first.");
    }

    const tokenAbiB = metadataB.abi;

    const tokenB = new web3.eth.Contract(tokenAbiB, tokenAddressB);

    const metadataDEX = JSON.parse(await remix.call('fileManager', 'getFile', 'contracts/artifacts/DEX.json'));
    if (!metadataDEX) {
        throw new Error("Could not find DEX.json artifact. Please compile the contract first.");
    }

    const tokenAbiDEX = metadataDEX.abi;

    const dex = new web3.eth.Contract(tokenAbiDEX, addressDEX);


    const metadataLP = JSON.parse(await remix.call('fileManager', 'getFile', 'contracts/artifacts/LPToken.json'));
    if (!metadataLP) {
        throw new Error("Could not find LPToken.json artifact. Please compile the LPToken contract first.");
    }

    const lpTokenAbi = metadataLP.abi;

    const lpTokenAddress = await dex.methods.lpToken().call();
    const lpToken = new web3.eth.Contract(lpTokenAbi, lpTokenAddress);

    // checks is user can add liquidity
    async function canAddLiquidity(user) {
        const userBalanceA = new BN(await tokenA.methods.balanceOf(user).call());
        const userBalanceB = new BN(await tokenB.methods.balanceOf(user).call());
        return userBalanceA.gt(new BN(0)) && userBalanceB.gt(new BN(0));
    }

    // checks iF user has TokenA
    async function hasA(user) {
        const userBalanceA = new BN(await tokenA.methods.balanceOf(user).call());
        return userBalanceA.gt(new BN(0));
    }

    // checks is user has TokenA
    async function hasB(user) {
        const userBalanceB = new BN(await tokenB.methods.balanceOf(user).call());
        return userBalanceB.gt(new BN(0));
    }


    async function simulateAddLiquidity(user) {
        const userBalanceA = await tokenA.methods.balanceOf(user).call();
        const userBalanceB = await tokenB.methods.balanceOf(user).call();

        const amountA = getRandomAmount(new BN(userBalanceA)); // pick a valid random amount of tokenA

        let reserves = await dex.methods.getReserves().call();
        let reserveA = new BN(reserves[0]);
        let reserveB = new BN(reserves[1]);

        const amountB = reserveA.gt(new BN(0)) && reserveB.gt(new BN(0))
            ? reserveB.mul(amountA).div(reserveA) // pick the amt of tokenB while maintaining the ratio 
            : getRandomAmount(new BN(userBalanceB)); //else its the first deposit so set the initial ratio by picking a random amt for tokenB

        await tokenA.methods.approve(dex.options.address, amountA.toString()).send({ from: user });
        await tokenB.methods.approve(dex.options.address, amountB.toString()).send({ from: user });


        const receipt = await dex.methods.addLiquidity(amountA.toString(), amountB.toString()).send({ from: user }); //add liquidity


        reserves = await dex.methods.getReserves().call();
        reserveA = new BN(reserves[0]);
        reserveB = new BN(reserves[1]);

        metrics.dexFeesA.push(metrics.dexFeesA[metrics.dexFeesA.length - 1]);
        metrics.dexFeesB.push(metrics.dexFeesB[metrics.dexFeesB.length - 1]);
        metrics.dexFees.push(metrics.dexFees[metrics.dexFees.length - 1]);

        const allBalances = await Promise.all(LPs.map(lp => lpToken.methods.balanceOf(lp).call()));
        const totalLP = allBalances.reduce((acc, val) => acc.add(new BN(val)), new BN(0));
        const dist = allBalances.map(val => parseFloat(val.toString()) / parseFloat(totalLP.toString()));
        const dist2 = allBalances.map(val => parseFloat(val.toString()));
        metrics.lpTokenDistributionRatio.push(dist);
        metrics.lpTokenDistribution.push(dist2);

        metrics.reserveRatios.push(parseFloat(reserveA.toString()) / parseFloat(reserveB.toString()));

        metrics.slippage.push(metrics.slippage[metrics.slippage.length - 1]);

        metrics.spotPrices.push(parseFloat(reserveA.toString()) / parseFloat(reserveB.toString()));

        metrics.swapVolumeA.push(metrics.swapVolumeA[metrics.swapVolumeA.length - 1]);
        metrics.swapVolumeB.push(metrics.swapVolumeB[metrics.swapVolumeB.length - 1]);

        const tvl = reserveA.muln(2);
        metrics.tvl.push(parseFloat(web3.utils.fromWei(tvl.toString())));
    }

    // checks is user can remove liquidity
    async function canRemoveLiquidity(user) {
        const userLP = new BN(await lpToken.methods.balanceOf(user).call());
        return userLP.gt(new BN(0));
    }


    async function simulateRemoveLiquidity(user) {
        const userLP = await lpToken.methods.balanceOf(user).call();
        const amount = getRandomAmount(new BN(userLP)); //pick a valid random amt of liquidity tokens to remove
        if (amount.gt(new BN(0))) {
            let reserves = await dex.methods.getReserves().call();
            let reserveA = new BN(reserves[0]);
            let reserveB = new BN(reserves[1]);

            const lpBalanceBefore = web3.utils.fromWei(userLP.toString());
            await dex.methods.removeLiquidity(amount.toString()).send({ from: user }); //remove liquidity 

            reserves = await dex.methods.getReserves().call();
            reserveA = new BN(reserves[0]);
            reserveB = new BN(reserves[1]);

            metrics.dexFeesA.push(metrics.dexFeesA[metrics.dexFeesA.length - 1]);
            metrics.dexFeesB.push(metrics.dexFeesB[metrics.dexFeesB.length - 1]);
            metrics.dexFees.push(metrics.dexFees[metrics.dexFees.length - 1]);

            const allBalances = await Promise.all(LPs.map(lp => lpToken.methods.balanceOf(lp).call()));
            const totalLP = allBalances.reduce((acc, val) => acc.add(new BN(val)), new BN(0));
            const dist = allBalances.map(val => parseFloat(val.toString()) / parseFloat(totalLP.toString()));
            const dist2 = allBalances.map(val => parseFloat(val.toString()));
            metrics.lpTokenDistributionRatio.push(dist);
            metrics.lpTokenDistribution.push(dist2);

            metrics.reserveRatios.push(parseFloat(reserveA.toString()) / parseFloat(reserveB.toString()));

            metrics.slippage.push(metrics.slippage[metrics.slippage.length - 1]);

            metrics.spotPrices.push(parseFloat(reserveA.toString()) / parseFloat(reserveB.toString()));

            metrics.swapVolumeA.push(metrics.swapVolumeA[metrics.swapVolumeA.length - 1]);
            metrics.swapVolumeB.push(metrics.swapVolumeB[metrics.swapVolumeB.length - 1]);

            const tvl = reserveA.muln(2);
            metrics.tvl.push(parseFloat(web3.utils.fromWei(tvl.toString())));

        }
    }


    async function simulateSwap(user) {
        let fromToken, toToken, reserveFrom, reserveTo;
        let isAtoB;

        if (!(await hasA(user))) { // if user has only tokenB then use it as input
            fromToken = tokenB;
            toToken = tokenA;
            reserveFrom = await dex.methods.reserveB().call();
            reserveTo = await dex.methods.reserveA().call();
            isAtoB = false;
        }
        else if (!(await hasB(user))) { // if user has only tokenA then use it as input
            fromToken = tokenA;
            toToken = tokenB;
            reserveFrom = await dex.methods.reserveA().call();
            reserveTo = await dex.methods.reserveB().call();
            isAtoB = true;
        }
        else { // else choose one of A and B as input uniformly randomly
            isAtoB = Math.random() > 0.5;
            if (isAtoB) {
                fromToken = tokenA;
                toToken = tokenB;
                reserveFrom = await dex.methods.reserveA().call();
                reserveTo = await dex.methods.reserveB().call();
            } else {
                fromToken = tokenB;
                toToken = tokenA;
                reserveFrom = await dex.methods.reserveB().call();
                reserveTo = await dex.methods.reserveA().call();
            }
        }

        const balance = await fromToken.methods.balanceOf(user).call();
        const reserveFromBN = new BN(reserveFrom);
        const reserveToBN = new BN(reserveTo);

        const amountIn = getRandomSwapAmount(new BN(balance), reserveFromBN);

        if (amountIn.gt(new BN(0))) {

            let reserves = await dex.methods.getReserves().call();
            let reserveA = new BN(reserves[0]);
            let reserveB = new BN(reserves[1]);


            await fromToken.methods.approve(dex.options.address, amountIn.toString()).send({ from: user });
            await dex.methods.swap(fromToken.options.address, amountIn.toString()).send({ from: user });


            const FEE_NUM = new BN(997);
            const FEE_DEN = new BN(1000);
            const inputAmountWithFee = amountIn.mul(FEE_NUM).div(FEE_DEN); //0.3 % collected as swap fee
            const fee = amountIn.sub(inputAmountWithFee);

            const numerator = reserveToBN.mul(reserveFromBN);
            const denominator = reserveFromBN.add(inputAmountWithFee);
            const amountOut = numerator.div(denominator); //output amt calculation while maintaining the product

            const expectedPrice = parseFloat(reserveToBN.toString()) / parseFloat(reserveFromBN.toString());
            const actualPrice = parseFloat(amountOut.toString()) / parseFloat(amountIn.toString());

            const slippage = ((actualPrice - expectedPrice) / expectedPrice) * 100;
            const symbolFrom = await fromToken.methods.symbol().call();
            const symbolTo = await toToken.methods.symbol().call();


            reserves = await dex.methods.getReserves().call();
            reserveA = new BN(reserves[0]);
            reserveB = new BN(reserves[1]);

            if (isAtoB) {
                const dexA = (metrics.dexFeesA[metrics.dexFeesA.length - 1] + parseFloat(fee.toString()));
                metrics.dexFeesA.push(dexA);
                const dexB = metrics.dexFeesB[metrics.dexFeesB.length - 1];
                metrics.dexFeesB.push(dexB);
                metrics.dexFees.push(dexA + dexB);

                const tokensA = (metrics.swapVolumeA[metrics.swapVolumeA.length - 1]) + (parseFloat(amountIn.toString()));
                const tokensB = (metrics.swapVolumeB[metrics.swapVolumeB.length - 1]) + (parseFloat(amountOut.toString()));
                metrics.swapVolumeA.push(tokensA);
                metrics.swapVolumeB.push(tokensB);
            }
            else {
                const dexA = metrics.dexFeesA[metrics.dexFeesA.length - 1];
                metrics.dexFeesA.push(dexA);
                const dexB = (metrics.dexFeesB[metrics.dexFeesB.length - 1]) + parseFloat(fee.toString());
                metrics.dexFeesB.push(dexB);
                metrics.dexFees.push(dexA + dexB);

                const tokensA = (metrics.swapVolumeA[metrics.swapVolumeA.length - 1]) + (parseFloat(amountOut.toString()));
                const tokensB = (metrics.swapVolumeB[metrics.swapVolumeB.length - 1]) + (parseFloat(amountIn.toString()));
                metrics.swapVolumeA.push(tokensA);
                metrics.swapVolumeB.push(tokensB);
            }

            metrics.lpTokenDistribution.push(metrics.lpTokenDistribution[metrics.lpTokenDistribution.length - 1]);
            metrics.lpTokenDistributionRatio.push(metrics.lpTokenDistributionRatio[metrics.lpTokenDistributionRatio.length - 1]);

            metrics.reserveRatios.push(parseFloat(reserveA.toString()) / parseFloat(reserveB.toString()));

            metrics.slippage.push(slippage);

            metrics.spotPrices.push(parseFloat(reserveA.toString()) / parseFloat(reserveB.toString()));

            const tvl = reserveA.muln(2);
            metrics.tvl.push(parseFloat(web3.utils.fromWei(tvl.toString())));

        }
    }


    // Distribute tokens
    try {
        for (let lp of LPs) {
            await tokenA.methods.transfer(lp, web3.utils.toWei("100")).send({ from: accounts[0] });
            await tokenB.methods.transfer(lp, web3.utils.toWei("200")).send({ from: accounts[0] });
        }
        for (let trader of traders) {
            await tokenA.methods.transfer(trader, web3.utils.toWei("62.5")).send({ from: accounts[0] });
            await tokenB.methods.transfer(trader, web3.utils.toWei("125")).send({ from: accounts[0] });
        }
        console.log("Initial token distribution done");
    } catch (error) {
        console.error("Error in token distribution:", error);
    }


    for (let i = 0; i < N; i++) {
        const reserves = await dex.methods.getReserves().call();
        const reserveA = new BN(reserves[0]);
        const reserveB = new BN(reserves[1]);

        if (reserveA.isZero() && reserveB.isZero()) { //if pool is empty then transaction is always addLiquidity
            const user = pickRandom([...LPs]);
            try {
                await simulateAddLiquidity(user);
            }
            catch (error) {
                console.log("1", error);
            }
        }
        else {
            const user = pickRandom([...LPs, ...traders]); // choose a random user and a corresponding transaction for them

            if (LPs.includes(user)) {
                if (!(await canAddLiquidity(user))) {
                    try {
                        await simulateRemoveLiquidity(user);
                    }
                    catch (error) {
                        console.log("2", error);
                    }
                }
                else if (!(await canRemoveLiquidity(user))) {
                    try {
                        await simulateAddLiquidity(user);
                    }
                    catch (error) {
                        console.log("1", error);
                    }
                }
                else if (Math.random() < 0.5) {
                    try {
                        await simulateAddLiquidity(user);
                    }
                    catch (error) {
                        console.log("1", error);
                    }
                } else {
                    try {
                        await simulateRemoveLiquidity(user);
                    }
                    catch (error) {
                        console.log("2", error);
                    }
                }
            } else {
                try { await simulateSwap(user);}
                catch (error) {
                    console.log("3", error);
                }
            }
        }
    }

    console.log("Final Metrics:");
    console.log(JSON.stringify(metrics, null, 2));

    console.log("Simulation complete");
})();
