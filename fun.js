function gen24Bits() {
   let out = "";
   for (let i = 0; i < 24; i++) {
      let bit = Math.round(Math.random());
      out += bit;
   }
   console.log(out);
}

for (let x = 0; x < 100; x++) {
   gen24Bits();
}

