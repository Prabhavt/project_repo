# Asynchronous FIFO design using verilog
	
	Author: Prabhav Talukdar
	
	An asynchronous FIFO (First In, First Out) is a type of buffer used to manage 
	data between systems with different clock domains. It ensures data integrity 
	by storing and synchronizing data across varying clock frequencies, using 
	control signals and pointers to handle data flow and avoid collisions or loss.

	[!image_info][https://www.google.com/url?sa=i&url=https%3A%2F%2Fvlsiverify.com%2Fverilog%2Fverilog-codes%2Fasynchronous-fifo%2F&psig=AOvVaw1vbQAG3_8_hGsB0s7NCdOi&ust=1723490317607000&source=images&cd=vfe&opi=89978449&ved=0CBEQjRxqFwoTCIiF5PLT7YcDFQAAAAAdAAAAABAQ]
	
	Modules:
		twoflopsynchronizer	->	Two FF synchronizer to facilitate CDC
		ptr_module		->	Handling write and read address pointer
		memory			->	Memory module handling data
		FIFO_top 		->	Top module integrating all sub-modules
	
	
