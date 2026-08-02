
let score = 10;
history(score);
score = 20;
score = 35;
score = 50;
history(score);


let distance = 5km + 500m;
show "Total distance in meters: " + distance;


benchmark {
    let total = 0;
    let i = 0;
    while (i < 1000) {
        total = total + i;
        i = i + 1;
    }
    show "Sum result inside benchmark: " + total;
}


let sales = 25;
draw_chart(sales);



let safe_balance = 1000;
show "Balance: " + safe_balance;

// Infinite Loop Protection
let loop_test = 0;
while (loop_test < 999999) {
    loop_test = loop_test + 1;
}
show "Loop stopped safely, final value: " + loop_test;



let_once secret_otp = 4521;
show "OTP (first read): " + secret_otp;
show "OTP (second read - EXPECTED TO CRASH HERE): " + secret_otp;