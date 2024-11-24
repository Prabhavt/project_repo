# Asynchronous FIFO design using verilog
	
	Author: Prabhav Talukdar
	
	An asynchronous FIFO (First In, First Out) is a type of buffer used to manage 
	data between systems with different clock domains. It ensures data integrity 
	by storing and synchronizing data across varying clock frequencies, using 
	control signals and pointers to handle data flow and avoid collisions or loss.

  ![asyncFIFO](https://github.com/user-attachments/assets/290545a2-666c-4ce8-b29b-c7ee42825100)


	Modules:
		twoflopsynchronizer	->	Two FF synchronizer to facilitate CDC
		ptr_module		->	Handling write and read address pointer
		memory			->	Memory module handling data
		FIFO_top 		->	Top module integrating all sub-modules

	Output signals:
![output_graph](https://github.com/user-attachments/assets/54ea3641-5377-4009-939d-204b2144ad9c)

	

	
