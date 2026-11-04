module crcmodule_tb;

	reg n_reset = 1;				// reg - what goes into the module
	reg srd = 0;					// the testbench drives the inputs
	reg swr = 0;
	reg clk = 0;
	reg [15:0] saddress = 0;
	reg [31:0] sdata_in = 0;

	wire [31:0] sdata_out;			// wire - what comes out of the module
									// the testbench observes how the module responds to inputs

	integer i;
	integer check = 8'h31;

	reg [31:0] state;				// last value read from STATE
	reg [31:0] result;				// last value read from RESULT
	integer errors = 0;

	// module registers (same as in crcmodule.v)
	localparam [15:0] IN = 16'h0200;
	localparam [15:0] STATE = 16'h0208;
	localparam [15:0] RESULT = 16'h0210;
	localparam [15:0] CTRL = 16'h0218;
	localparam [31:0] calculate_CRC = 32'h00000000;

always #1 clk = ~clk;

initial begin
$dumpfile("crcmodule.vcd");
$dumpvars(0, crcmodule_tb);
end


//============ Bus (same contract as the QEMU device) ============//

	// CPU write: one clk cycle with swr = 1
	task bus_write(input [15:0] addr, input [31:0] data);
	begin
		@(negedge clk);
		saddress = addr;
		sdata_in = data;
		swr = 1;
		@(negedge clk);
		swr = 0;
	end
	endtask

	// CPU read: one clk cycle with srd = 1, then sample sdata_out
	task bus_read(input [15:0] addr, output [31:0] data);
	begin
		@(negedge clk);
		saddress = addr;
		srd = 1;
		@(negedge clk);
		srd = 0;
		data = sdata_out;
	end
	endtask

	// compare read value with expected one
	task expect_value(input [31:0] got, input [31:0] expected);
	begin
		if (got === expected)
			$display("        OK    0x%04X", got[15:0]);
		else begin
			$display("        FAIL  0x%04X (expected 0x%04X)", got[15:0], expected[15:0]);
			errors = errors + 1;
		end
	end
	endtask

//=================================================================


initial begin

	//======== Example of correct operation ============//

	// RESET
	# 1 n_reset = 0;
	# 1 n_reset = 1;

	// Data input
	for (i = 0; i < 9; i = i + 1) begin // crc = 0xAD30
		bus_write(IN, i);
	end

	// Calc CRC
	bus_write(CTRL, calculate_CRC);

	// crc state
	bus_read(STATE, state);
	$display("[0] STATE  after CTRL: 0x%02X (calculating)", state[7:0]);

	// give result
	repeat (10) @(negedge clk);
	bus_read(RESULT, result);
	$display("[0] RESULT bytes 0x00..0x08:");
	expect_value(result, 32'h0000AD30);

	// crc state
	bus_read(STATE, state);
	$display("[0] STATE  result ready:");
	expect_value(state, 32'h00000009);


	//===============================================//

	// RESET
	# 1 n_reset = 0;
	# 1 n_reset = 1;

	//======== Checking various scenarios ===============//


	// 1) writing data during CRC calculation - ok, the module ignores the extra data

	// Data input
	for (i = 0; i < 9; i = i + 1) begin // crc = 0x4C06
		bus_write(IN, check + i);
	end

	// Calc CRC
	bus_write(CTRL, calculate_CRC);

	// crc state
	bus_read(STATE, state);
	$display("[1] STATE  after CTRL: 0x%02X (calculating)", state[7:0]);

	// Data input
	bus_write(IN, 20);

	// give result
	bus_read(RESULT, result);
	$display("[1] RESULT during calculation: 0x%04X (not ready yet)", result[15:0]);

	// crc state
	bus_read(STATE, state);
	$display("[1] STATE  during calculation: 0x%02X", state[7:0]);

	// give result
	repeat (10) @(negedge clk);
	bus_read(RESULT, result);
	$display("[1] RESULT \"123456789\" (extra byte ignored):");
	expect_value(result, 32'h00004C06);

	// crc state
	bus_read(STATE, state);
	$display("[1] STATE  result ready:");
	expect_value(state, 32'h00000009);


	//===============================================//

	// RESET
	# 1 n_reset = 0;
	# 1 n_reset = 1;

	// 2) CRC calculation without data - ok, the output shows the init value 0xFFFF

	// Calc CRC
	bus_write(CTRL, calculate_CRC);

	// give result
	bus_read(RESULT, result);
	$display("[2] RESULT without data:");
	expect_value(result, 32'h0000FFFF);

	// crc state
	bus_read(STATE, state);
	$display("[2] STATE  without data (CTRL ignored):");
	expect_value(state, 32'h00000001);


	//===============================================//

	// RESET
	# 1 n_reset = 0;
	# 1 n_reset = 1;

	// 3) Buffer overflow (more than 250 input bytes)

	// Data input
	for (i = 0; i < 255; i = i + 1) begin // only 250 bytes accepted - crc = 0x42C9
		bus_write(IN, i);
	end

	// crc state
	bus_read(STATE, state);
	$display("[3] STATE  buffer full:");
	expect_value(state, 32'h00000000);

	// Calc CRC
	bus_write(CTRL, calculate_CRC);

	// give result
	repeat (10) @(negedge clk);
	bus_read(RESULT, result);
	$display("[3] RESULT during calculation: 0x%04X (not ready yet)", result[15:0]);

	// crc state
	repeat (250) @(negedge clk);
	bus_read(STATE, state);
	$display("[3] STATE  result ready:");
	expect_value(state, 32'h00000009);

	// give result
	bus_read(RESULT, result);
	$display("[3] RESULT bytes 0x00..0xF9 (250 bytes):");
	expect_value(result, 32'h000042C9);

	repeat (10) @(negedge clk);

	if (errors == 0)
		$display("\nALL TESTS PASSED");
	else
		$display("\n%0d TEST(S) FAILED", errors);

	$finish;

end

crcmodule crc_instance(n_reset, saddress, srd, swr, sdata_in, sdata_out, clk);

endmodule
