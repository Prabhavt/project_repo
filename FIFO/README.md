# Asynchronous FIFO design using verilog
	
	Author: Prabhav Talukdar
	
	An asynchronous FIFO (First In, First Out) is a type of buffer used to manage 
	data between systems with different clock domains. It ensures data integrity 
	by storing and synchronizing data across varying clock frequencies, using 
	control signals and pointers to handle data flow and avoid collisions or loss.

	
	Modules:
		twoflopsynchronizer	->	Two FF synchronizer to facilitate CDC
		ptr_module		->	Handling write and read address pointer
		memory			->	Memory module handling data
		FIFO_top 		->	Top module integrating all sub-modules
	
	
