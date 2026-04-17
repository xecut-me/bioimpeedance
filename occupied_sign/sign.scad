$fn = 128;
// $fa = 12;

body_w = 100;
body_h = 100;
body_t = 20;
body_r = 10;

wall_t = 3;
wall_r = body_r - wall_t;

transparent_t = 0.2;
solid_t = wall_t;

channel_offset = 5;
channel_d = 3;
aux_channel_offset = 15;
aux_channel_d = 5;

difference() {
    // body bulk
    translate([body_r, body_r, 0])
    linear_extrude(body_t)
    minkowski() {
        square([body_w - (2 * body_r), body_h - (2 * body_r)]);
        circle(body_r);
    };

    // cavity sections
    for(section_yh = [[0, 30], [33, 30], [66, 28]]) {
        section_y = section_yh[0] + body_r;
        section_h = section_yh[1];
        section_w = body_w - (2 * wall_t);

        translate([body_r, section_y, solid_t])
        linear_extrude(body_t)
        minkowski() {
            square([section_w - (2 * wall_r), section_h - (2 * wall_r)]);
            circle(wall_r);
        };
    }

    // cable channel
    translate([channel_offset, wall_t * 2, body_t - channel_d - 2]) 
    rotate([-90, 0, 0])
    cylinder(h = body_h, r = channel_d / 2);

    // auxiliary external channel
    translate([body_w - aux_channel_offset, body_h - wall_t - 0.5, body_t - aux_channel_d - 2]) 
    rotate([-90, 0, 0])
    cylinder(h = wall_t + 1, r = aux_channel_d / 2);

    // text
    for(item = [["Toilet is:", 74, 13], ["Occupied", 41, 13], ["Free", 8, 13]]) {
        label = item[0];
        offs = item[1];
        size = item[2];
        translate([0, 0, transparent_t])
        linear_extrude(10)
        translate([body_w / 2, offs + body_r])
        rotate([0, 180, 0])
        text(label, font = "Roboto Slab", size = size, halign = "center", valign = "center");
    }
}
