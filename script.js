// let vocDataElement = document.getElementById('vocData');
// let errorMessageElement = document.getElementById('errorMessage');

// const SERVICE_UUID = '12345678-1234-5678-1234-56789abcdef0';
// const CHARACTERISTIC_UUID = 'abcd1234-5678-1234-5678-abcdef123456';

// async function connectToDevice() {
//   try {
//     const device = await navigator.bluetooth.requestDevice({
//       filters: [{ services: [SERVICE_UUID] }],
//       optionalServices: [SERVICE_UUID],
//     });

//     const server = await device.gatt.connect();

//     const service = await server.getPrimaryService(SERVICE_UUID);

//     const characteristic = await service.getCharacteristic(CHARACTERISTIC_UUID);

//     await characteristic.startNotifications();
    
//     characteristic.addEventListener('characteristicvaluechanged', handleCharacteristicValueChanged);

//     vocDataElement.textContent = "Connected to ESP32!";
//   } catch (error) {

//     errorMessageElement.textContent = `Error: ${error}`;
//   }
// }

// function handleCharacteristicValueChanged(event) {
//   const value = event.target.value;
//   const vocData = new TextDecoder().decode(value); 
//   vocDataElement.textContent = `VOC Level: ${vocData}`;
// }

// document.getElementById('connect').addEventListener('click', connectToDevice);