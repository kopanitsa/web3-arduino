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
3. Install board settings [ESP32-Arduino](https://github.com/espressif/arduino-esp32)
4. `Sketch` -> `Include Library` -> `Add .ZIP file` -> select downloaded zip file.
5. Then you can use this from Arduino IDE.

## Example

Please refer `examples` directory to learn more about how to use the libraries

### Setup

```C++
#define INFURA_HOST "rinkeby.infura.io"
#define INFURA_PATH "/<YOUR_INFURA_ID>"

Web3 web3(INFURA_HOST, INFURA_PATH);
```
### Generate a private key
```python -c 'import os; print ", ".join([ str(ord(c)) for c in bytes(os.urandom(32)) ])'```

Copy this to terminal and paste the output as your private key in the ino file.

To convert this to a private key you can use in MyEtherWallet you do: 
```print ''.join('{:02x}'.format(x) for x in array_alpha)```

## Dependencies

- [cJSON](https://github.com/DaveGamble/cJSON)
- [secp256k1](https://github.com/bitcoin-core/secp256k1)
- [ESP32-Arduino](https://github.com/espressif/arduino-esp32)

