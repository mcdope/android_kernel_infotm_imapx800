	// 0: video burst mode , 24bpp , 320x480
	{
        1, // no_of_lanes
	 0, // virtual_channel
	 VIDEO_BURST_WITH_SYNC_PULSES, //video_mode
	 1, //receive_ack_packets
	 27000, //byte_clock
	 6750, //pixel_clock
	 COLOR_CODE_24BIT, //color_coding
	 0, //is_18_loosely
	 1, //data_en_polarity
	 1, // h_polarity
	 320, // h_active_pixels
	 5, // h_sync_pixels
	 5, // h_back_porch_pixels
	 355,// h_total_pixels
	 1, //v_polarity
  	 480, // v_active_lines
	 1, // v_sync_lines
	 7, // v_back_porch_lines
	 496 // v_total_lines
	},
	//	1 : video non burst mode 24bpp, 320x480
	{
    	 1, // no_of_lanes
	 0, // virtual_channel
	 VIDEO_NON_BURST_WITH_SYNC_EVENTS, //video_mode
	 1, //receive_ack_packets
	 27000, //byte_clock
	 6750, //pixel_clock
	 COLOR_CODE_24BIT, //color_coding
	 0, //is_18_loosely
	 1, //data_en_polarity
	 1, // h_polarity
	 320, // h_active_pixels
	 5, // h_sync_pixels
	 5, // h_back_porch_pixels
	 355, // h_total_pixels
	 1, //v_polarity
        480, // v_active_lines
	 1, // v_sync_lines
	 7, // v_back_porch_lines
	 496 // v_total_lines
	}
