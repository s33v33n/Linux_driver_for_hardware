/*	Verilog module (hardware) controlled by the CPU inside the QEMU emulator */

//========== Inputs, Outputs ===========================//

module crcmodule(

	input  			n_reset, 			// asynchronous reset, active low
	input  [15:0] 	saddress,		 	// address to read from / write to  /* ADDRESS BUS */

	input  			srd,				// read strobe	/* CONTROL BUS */
	input  			swr, 				// write strobe

	input  [31:0] 	sdata_in, 			// input data bus, data from the CPU /* DATA BUS */
	output [31:0] 	sdata_out, 			// output data bus, data to the CPU

	input  			clk 				// clock signal
);

//=========================================================


//============ Internal signals ===========================

	reg [7:0]	data_buf [0:249]; 		// register holding the user's input data

	reg [15:0] crc = 16'hFFFF;			// checksum register (CRC16)
	reg [15:0] crc_result = 0;			//

	reg [3:0] crc_state = 4'b0001;		// Event bits -> [0] = Data to process: 0 - buffer full or CRC calculation started; 1 - write allowed
										//				[1] = CRC start (first step - initialization): 0 - no init; 1 - load data and move on to CRC calculation
										//				[2] = CRC calculation: 0 - cannot calculate; 1 - calculation running
										// 				[3] = Is the CRC result ready? 0 - no; 1 - yes

	reg [31:0] sdata_out_s = 0;

	//=========================================================

	reg [15:0] LUT_table [0:255];

	// LUT generated at start-up (CRC16-CDMA2000, poly 0xC867)
	integer lut_i, lut_b;
	reg [15:0] lut_val;

	initial begin
		for (lut_i = 0; lut_i < 256; lut_i = lut_i + 1) begin
			lut_val = {lut_i[7:0], 8'h00};
			for (lut_b = 0; lut_b < 8; lut_b = lut_b + 1) begin
				if (lut_val[15])
					lut_val = (lut_val << 1) ^ 16'hC867;
				else
					lut_val = lut_val << 1;
			end
			LUT_table[lut_i[7:0]] = lut_val;
		end
	end

	reg [7:0] i = 0;					// number of bytes in data_buf (0-250)
	reg [7:0] bytes = 0;				// number of bytes to process
	reg [7:0] byte_num = 0;				// currently processed byte

	wire [7:0] index = crc[15:8] ^ data_buf[byte_num];	// LUT index for the current byte

	localparam [15:0] IN = 16'h0200;
	localparam [15:0] STATE = 16'h0208;
	localparam [15:0] RESULT = 16'h0210;
	localparam [15:0] CTRL = 16'h0218;
	localparam [31:0] calculate_CRC = 32'h00000000;


/******* One clock domain: CPU RESET, CRC, CPU READ, CPU WRITE *******/
// Bus contract (QEMU device / testbench):
//   write: set saddress + sdata_in, swr = 1 for exactly one clk cycle
//   read:  set saddress, srd = 1 for exactly one clk cycle, then sample sdata_out
always @(posedge clk or negedge n_reset) begin

	/******* CPU RESET Verilog *******/
	if (!n_reset) begin
		sdata_out_s <= 0;
		crc_result <= 0;
		crc_state <= 4'b0001;
		i <= 0;
	end

	else begin

		/******* CRC calculation *******/
		if (crc_state[1]) begin		// init
			crc <= 16'hFFFF;
			crc_result <= 16'hFFFF;
			byte_num <= 0;
			bytes <= i;
			crc_state[2:1] <= 2'b10;
		end

		else if (crc_state[2]) begin	//count CRC

			if (byte_num < bytes) begin
				crc <= (crc << 8) ^ LUT_table[index];
				byte_num <= byte_num + 1;
			end
			else begin
				crc_result <= crc;			// result valid together with ready bit
				crc_state[3:2] <= 2'b10;
			end
		end

		else if (crc_state[3]) begin 	// CRC ready
			crc_result <= crc;
			crc_state[0] <= 1'b1;
			i <= 0;
		end


		/******* CPU READ from Verilog *******/ /* sdata_out <- STATE / RESULT */
		if (srd) begin
			sdata_out_s <= 0;

			if (saddress == STATE) begin
				sdata_out_s[3:0] <= crc_state;
			end

			if (saddress == RESULT) begin
				if (crc_state[3] == 1) begin
					sdata_out_s <= {16'h0000, crc_result};
				end
				else begin
					sdata_out_s <= 32'h0000FFFF;
				end
			end
		end


		/******* CPU WRITE to Verilog *******/ // sdata_in -> IN / CTRL
		// placed last: in the same cycle CPU write has priority over CRC logic
		if (swr) begin
			if (saddress == IN) begin		// Load user data
				if (crc_state[0] == 1) begin
					data_buf[i] <= sdata_in[7:0];
					i <= i + 1;
					crc_state[3] <= 1'b0;

					if(i == 249) begin 	// Data block full
						crc_state[0] <= 1'b0;
					end
				end
			end

			if (saddress == CTRL) begin 	// Calculate CRC
				if ( i > 0 && crc_state[2:1] == 2'b00) begin 	// ignored while calculating
					if(sdata_in == calculate_CRC) begin
						crc_state[1:0] <= 2'b10;
					end
				end
			end
		end

	end
end

	assign sdata_out = sdata_out_s;

endmodule
