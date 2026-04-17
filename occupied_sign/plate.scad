$fn = 128;
// $fa = 12;

body_w = 100;
body_h = 100;
body_t = 2;
body_r = 10;

translate([body_r, body_r, 0])
linear_extrude(body_t)
minkowski() {
    square([body_w - (2 * body_r), body_h - (2 * body_r)]);
    circle(body_r);
};
