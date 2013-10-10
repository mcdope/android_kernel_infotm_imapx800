// 0 :  8bit/18bpp  6+6+6 , 240x320
{
	1, // no_of_lanes;
	27000, // byte_clock;
	0, // dbi_vid;
	3, // out_dbi_conf;
	1, // partitioning_en;
	1, // lut_size_conf;
	3, // in_dbi_conf;
	721, // allowed_cmd_size;
	721, // wr_cmd_size;
	1 // receive_ack_packets;
},
// 1 :  240x320 .  8-wire,  8,     8bpp; lane num = 1;
{
	1, // no_of_lanes;
	27000, // byte_clock;
	0, // dbi_vid;
	0, // out_dbi_conf;
	1, // partitioning_en;
	1, // lut_size_conf;
	0, // in_dbi_conf;
	241, // allowed_cmd_size;
	241, // wr_cmd_size;
	1 // receive_ack_packets;
}
