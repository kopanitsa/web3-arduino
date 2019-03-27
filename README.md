## unmaintained

Sorry but I cannot take time to maintain the library now...

## web3-arduino

![img](https://user-images.githubusercontent.com/891384/36104056-4994f600-1054-11e8-94f4-9f067610a6bf.png)

- What is this library?
    - This is an Arduino (or ESP32) library to use web3 on Ethereum platform.

- What is Arduino? 
    - Arduino is an open source computer hardware and software.
    - https://www.arduino.cc/
- What is ESP32?
    - ESP32 is a series of low cost, low power system on a chip microcontrollers with integrated Wi-Fi and dual-mode Bluetooth. 
    - https://www.espressif.com/en/products/hardware/esp32/overview    
- What is web3?
    - Web3 is the Ethereum compatible API which implements the Generic JSON RPC spec. Originally Javascript version is developed.
    - https://github.com/ethereum/web3.js/
- What is Ethereum?
    - Ethereum is a decentralized platform for applications that run exactly as programmed without any chance of fraud, censorship or third-party interference.
    - https://www.ethereum.org/
    
## Environment

- Confirmed device
    - ESP-WROOM-32
- Used Ethereum client
    - INFURA (https://infura.io)

## Installation

1. download the zip file from `Clone or download` button on Github.
2. launch Arduino IDE.
3. `Sketch` -> `Include Library` -> `Add .ZIP file` -> select downloaded zip file.
4. Then you can use this from Arduino IDE.

## Example

Please refer `examples` directory.

### setup

```C++
#define INFURA_HOST "rinkeby.infura.io"
#define INFURA_PATH "/<YOUR_INFURA_ID>"

Web3 web3(INFURA_HOST, INFURA_PATH);
```

### call web3 methods

```C++
char result[128];

web3.Web3ClientVersion(result);
USE_SERIAL.println(result);

web3.Web3Sha3("0x68656c6c6f20776f726c64", result);
USE_SERIAL.println(result);
```

### `call` to Contract

```C++
Contract contract(&web3, CONTRACT_ADDRESS);
strcpy(contract.options.from, MY_ADDRESS);
strcpy(contract.options.gasPrice,"2000000000000");
contract.options.gas = 5000000;
contract.SetupContractData(result, "get()");
contract.Call(result);
USE_SERIAL.println(result);
```

### `sendTransaction` to Contract

```C++
Contract contract(&web3, CONTRACT_ADDRESS);
contract.SetPrivateKey((uint8_t*)PRIVATE_KEY);
uint32_t nonceVal = (uint32_t)web3.EthGetTransactionCount((char *)MY_ADDRESS);

uint32_t gasPriceVal = 141006540;
uint32_t  gasLimitVal = 3000000;
uint8_t toStr[] = CONTRACT_ADDRESS;
uint8_t valueStr[] = "0x00";
uint8_t dataStr[100];
memset(dataStr, 0, 100);
contract.SetupContractData((char*)dataStr, "set(uint256)", 123);
contract.SendTransaction((uint8_t *) result,
                         nonceVal, gasPriceVal, gasLimitVal, toStr, valueStr, dataStr);

USE_SERIAL.println(result);
```

## Dependency

- [cJSON](https://github.com/DaveGamble/cJSON)
- [secp256k1](https://github.com/bitcoin-core/secp256k1)
- [ESP32-Arduino](https://github.com/espressif/arduino-esp32)

