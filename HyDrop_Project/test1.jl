let a = 10;
let b = 3;
show "\nHello, World!\n";
show "a = " + a;
show "b = " + b;

let sum_result = a + b;
show "a + b = " + sum_result;

let sub_result = a - b;
show "a - b = " + sub_result;

let mul_result = a * b;
show "a * b = " + mul_result;

let div_result = a / b;
show "a / b = " + div_result;

let is_greater = a > b;
show "a > b = " + is_greater;

let is_smaller = a < b;
show "a < b = " + is_smaller;

let is_equal = a == b;
show "a == b = " + is_equal;

let is_not_equal = a != b;
show "a != b = " + is_not_equal;

let is_greater_equal = a >= b;
show "a >= b = " + is_greater_equal;

let is_smaller_equal = a <= b;
show "a <= b = " + is_smaller_equal;

if (a > b) {
    show "if-branch: a is bigger -> " + a;
} else {
    show "else-branch: b is bigger -> " + b;
}

if (a == b) {
    show "if-branch: equal -> " + a;
} else {
    show "else-branch: not equal, sum was -> " + sum_result;
}

let counter = 0;
while (counter < 5) {
    show "counter = " + counter;
    counter = counter + 1;
}

let x = 100;
x = x - 25;
show "x after subtraction = " + x;

input user_age;
show "Your age: " + user_age;

let double_age = user_age * 2;
show "Double age: " + double_age;

input user_score;
if (user_score > 50) {
    show "Passed! Score: " + user_score;
} else {
    show "Failed! Score: " + user_score;
}


show "Line 1\nLine 2";
show "Name:\tJoy\nAge:\t25";