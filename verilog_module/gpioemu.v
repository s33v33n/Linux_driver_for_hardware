/*	Modul Verilog (hardware) sterowany przez CPU wewnatrz emulatora QEMU */

module gpioemu(n_reset,
    saddress[15:0], srd, swr,
    sdata_in[31:0], sdata_out[31:0],
	gpio_in[31:0], gpio_latch,
	gpio_out[31:0],
	clk,
	gpio_in_s_insp[31:0]);

	/* verilator lint_off UNUSED */
	/* verilator lint_off UNDRIVEN */
	/* verilator lint_off MULTIDRIVEN */
	/* verilator lint_off CASEINCOMPLETE */
	/* verilator lint_off COMBDLY */
	/* verilator lint_off WIDTH */

//========== Wejscia , Wyjscia ==========================//

	input  			n_reset; 			// reset stanow wyjsciowych
	input  [15:0] 	saddress;		 	// adres z ktorego odczytujemy/zapisujemy dane  /* MAGISTRALA ADRESOWA */

	input  			srd;				// polecenie odczytu	/* MAGISTRALA STERUJACA */
    input  			swr; 				// polecenie zapisu
    	
	input  [31:0] 	sdata_in; 			// magistrala odczytywanych danych, dane od procesora /* MAGISTRALA DANYCH*/
	output [31:0] 	sdata_out; 			// magistrala zapisywanych danych, dane do procesora 
		
	input  [31:0] 	gpio_in; 			// dane z peryferiow - wejsciowe (CPU przekazuje dane)
    input  			gpio_latch; 		// zatrzask danych z peryferiow 	
    output [31:0] 	gpio_out;			// dane do zapisu w peryferiach - wyjsciowe (CPU odczytuje dane)

	input  			clk; 				// sygnał clk
	output [31:0] 	gpio_in_s_insp;		// do testowania
 
//=========================================================


//============ Sygnaly pomocniczne ========================	
	
	reg [31:0]  gpio_in_s = 0;			// rejestr pomocniczy utrzymujący stan wejściowego gpio
	reg [31:0] 	gpio_out_s = 0;			// rejestr pomocniczy do aktualizowania stanu wyjściowego gpio (tylko ostatni zapis)
	
	reg [7:0]	data_buf [0:249]; 		// rejestr przechowujący dane wejsciowe uzytkownia 
	
	reg [31:0] crc = 32'h0000FFFF;		// rejestr do operacji sumy kontrolnej, 32 bity bo magistrala danych wyjsciowych jest 32-bitowa 
	reg [15:0] crc_result = 0;			//
	
	reg [3:0] crc_state = 4'b0001;		// Eventbits -> [0] = Dane do przetworzenia , 0 - brak pamieci lub rozpoczeta kalkulacja CRC ; 1 - mozliwy zapis 
										//				[1] = Rozpoczecie liczenia CRC (pierwszy krok - inicjalizacja) , 0 - brak danych init ; 1 - wprowadzenie danych i przejscie do liczenia CRC
										//				[2] = Rozpoczecie liczenia CRC , 0 - nie mozna liczyc ; 1 - rozpoczecie liczenia
										// 				[3] = Czy wynik CRC jest gotowy? 0 - nie ; 1 - tak 	
	
	reg [31:0] sdata_out_s = 0;
	reg [7:0] index = 0;
	
	//reg [31:0]	reg_test = 0;			// DO TESTOW
	
	//=========================================================

	reg [15:0] LUT_table [0:255];
	
	initial begin
    $readmemh("lut.mem", LUT_table);
	end

	integer i = 0;
	integer bytes = 0;
	integer byte_num = 0;
	
	`define IN 16'h0200
	`define STATE 16'h0208
	`define RESULT 16'h0210
	`define CTRL 16'h0218
	`define calculate_CRC 32'h00000000
	
	//`define TEST 16'h0250


/******* CPU RESET Verilog *******/
always @ (negedge n_reset) begin
	gpio_out_s <= 0;
	gpio_in_s <= 0;
	sdata_out_s <= 0;
	crc_result <= 0;
	crc_state <= 4'b0001;
	i <= 0;

end


/******* GPIO LATCH data from Verilog to CPU *******/
always @ (posedge gpio_latch) begin
	gpio_in_s <= gpio_in;
end


/******* CPU READ from Verilog *******/ /* data_out <- gpio_in */
always@(posedge srd) begin 

	sdata_out_s <= 0;
	
	if (saddress == `STATE) begin 
		sdata_out_s[3:0] <= crc_state;
	
	end

	if (saddress == `RESULT) begin
		if (crc_state[3] == 1) begin 
			sdata_out_s <= crc_result;
		end

		else begin 
			sdata_out_s <= 32'h0000FFFF;
		end 
	end

	//if (saddress == `TEST) begin // DO TESTOW
	//	sdata_out_s <= reg_test;
	//end 
	
end


/******* CPU WRITE to Verilog *******/ // data_in -> gpio_out 
always@(posedge swr) begin
	if (saddress == `IN) begin		// Ladowanie danych użytkownika 
		
		if (crc_state[0] == 1) begin 
			data_buf[i] <= sdata_in[7:0];
			gpio_out_s[7:0] <= sdata_in[7:0];
			i <= i + 1;
			crc_state[3] <= 1'b0;
			
			if(i == 249) begin 	// Zapelnienie bloku danych 
				crc_state[0] <= 1'b0;
				//i <= 249;
			end
		end 
		
	end
	
	if (saddress == `CTRL) begin 	// Oblicz CRC 
		
		if ( i > 0) begin 
			if(sdata_in == `calculate_CRC) begin
				crc_state[1:0] <= 2'b10;
				
			end
		end 
	end 
	
	//if (saddress == `TEST) begin 		// DO TESTOW
	//	reg_test <= sdata_in;
	//end 
end

always @(posedge clk) begin
	if (crc_state[1]) begin		// init 
		crc <= 32'h0000FFFF;
		crc_result <= 32'h0000FFFF;
		byte_num <= 0;
		bytes <= i;
		crc_state[2:1] <= 2'b10;  
	end
	
	else if (crc_state[2]) begin	//count CRC
		
		if (byte_num < bytes) begin
		
			/* verilator lint_off BLKSEQ */
			index = crc[15:8] ^ data_buf[byte_num];
			/* verilator lint_on BLKSEQ */
			
            crc <= (crc << 8) ^ {16'h0000,LUT_table[index]};
			
			byte_num <= byte_num + 1;
		end 
		else begin
			crc_state[3:2] <= 2'b10;
		end
	end

	else if (crc_state[3]) begin 	// CRC ready 	
		crc_result <= crc;
		crc_state[0] <= 1'b1;
		i <= 0;

	end

end

	assign sdata_out = sdata_out_s;
	assign gpio_out = gpio_out_s;
	assign gpio_in_s_insp = gpio_in_s;

endmodule 


