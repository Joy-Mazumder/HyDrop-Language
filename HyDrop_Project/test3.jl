
let x = 10;
reactive y = x * 2;
show "y (should be 20) = " + y;
x = 5;
show "x changed to 5. y auto-updated (should be 10) = " + y;


money bill = 0.1 + 0.2;
show "bill (should be exactly 0.30) = " + bill;


let divisor = 0;
retry(3) {
    let result = 100 / divisor;
    show "This should not print: " + result;
} on_fail {
    show "All retry attempts failed as expected.";
}


// ===== for loop check =====
for (let i = 0; i < 5; i = i + 1) {
    show "for-loop i = " + i;
}