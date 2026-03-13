module gpioemu_tb;

	reg n_reset = 1;				// reg - to co wchodzi do modułu 
	reg srd = 0;					// testbench steruje inputami 
	reg swr = 0;
	reg gpio_latch = 0;
	reg clk = 0;
	reg [15:0] saddress = 0;
	reg [31:0] sdata_in = 0;
	reg [31:0] gpio_in = 0;
	
	wire [31:0] sdata_out;			// wire - to co wychodzi z modułu 
	wire [31:0] gpio_out;			// testbench obserwuje jak modul odpowiada na inputy 
	wire [31:0] gpio_in_s_insp;
	
	integer i;
	integer check = 8'h31;
	
always #1 clk = ~clk;
	
initial begin 
$dumpfile("gpioemu.vcd"); 
$dumpvars(0, gpioemu_tb); 
end

initial begin

	//======== Przykladowe poprawne dzialanie ==========//

	// RESET
	# 1 n_reset = 0;
	# 1 n_reset = 1;
	
	// Data input
	# 1 saddress = `IN;

	for (i = 0; i < 9; i++) begin // crc = 0xAD30 
		# 1 sdata_in[7:0] = i;  
		# 1  swr = 1;
		# 1  swr = 0;
	end
	
	// Calc CRC
	# 1 saddress = `CTRL;
	# 1 sdata_in = `calculate_CRC;
	# 1  swr = 1;
	# 1  swr = 0;
	
	
	// crc state
	# 1 saddress = `STATE; 
	# 1  srd = 1;
	# 1  srd = 0;
	
	
	// give result 
	# 20 saddress = `RESULT;
	# 1  srd = 1;
	# 1  srd = 0;
	
	
	// crc state
	# 1 saddress = `STATE; 
	# 1  srd = 1;
	# 1  srd = 0;
	
	 
	//===============================================//
	
	// RESET
	# 1 n_reset = 0;
	# 1 n_reset = 1;
	
	//======== Sprawdzanie roznych scenariuszy ==========//
	
	
	// 1) wpisanie danych podczas liczenia CRC - ok , modul nie reaguje na dodatkowe dane 
	
	// Data input
	# 1 saddress = `IN;

	for (i = 0; i < 9; i++) begin // crc = 0x4C06 
		# 1 sdata_in[7:0] = check + i;  
		# 1  swr = 1;
		# 1  swr = 0;
	end
	
	// Calc CRC
	# 1 saddress = `CTRL;
	# 1 sdata_in = `calculate_CRC;
	# 1  swr = 1;
	# 1  swr = 0;
	
	// crc state
	# 1 saddress = `STATE; 
	# 1  srd = 1;
	# 1  srd = 0;
	
	// Data input
	# 1 saddress = `IN;
	
	# 1 sdata_in[7:0] = 20;  
	# 1  swr = 1;
	# 1  swr = 0;
	
	// give result 
	# 1 saddress = `RESULT;
	# 1  srd = 1;
	# 1  srd = 0;
	
	// crc state
	# 1 saddress = `STATE; 
	# 1  srd = 1;
	# 1  srd = 0;
	
	// give result 
	# 20 saddress = `RESULT;
	# 1  srd = 1;
	# 1  srd = 0;
	
	// crc state
	# 1 saddress = `STATE; 
	# 1  srd = 1;
	# 1  srd = 0;
	
	
	//===============================================//
	
	// RESET
	# 1 n_reset = 0;
	# 1 n_reset = 1;
	
	// 2) liczenie crc bez danych - ok, na wyjsciu pojawia sie wartosc init - 0xFFFF 
	
	// Calc CRC
	# 1 saddress = `CTRL;
	# 1 sdata_in = `calculate_CRC;
	# 1  swr = 1;
	# 1  swr = 0;
	
	// give result 
	# 2 saddress = `RESULT;
	# 1  srd = 1;
	# 1  srd = 0;
	
	// crc state
	# 1 saddress = `STATE; 
	# 1  srd = 1;
	# 1  srd = 0;
	
	
	//===============================================//
	
	// RESET
	# 1 n_reset = 0;
	# 1 n_reset = 1;
	
	// 3) Zpełnienie bufora (wiecej niz 250 bajtow wejsciowych)
	
	// Data input
	# 1 saddress = `IN;

	for (i = 0; i < 255; i++) begin // crc = 0x4C06 
		# 1 sdata_in[7:0] = i;  
		# 1  swr = 1;
		# 1  swr = 0;
	end
	
	// Calc CRC
	# 1 saddress = `CTRL;
	# 1 sdata_in = `calculate_CRC;
	# 1  swr = 1;
	# 1  swr = 0;
	
	// give result 
	# 20 saddress = `RESULT;
	# 1  srd = 1;
	# 1  srd = 0;
	
	// crc state
	# 500 saddress = `STATE; 
	# 1  srd = 1;
	# 1  srd = 0;
	
	// give result 
	# 1 saddress = `RESULT;
	# 1  srd = 1;
	# 1  srd = 0;
		
	# 1000 $finish;

end

gpioemu instancja(n_reset, saddress, srd, swr, sdata_in, sdata_out, gpio_in, gpio_latch, gpio_out, clk, gpio_in_s_insp); 

endmodule
