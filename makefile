# Copyright 2022

# This file is part of QUANTAS.

# QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

# QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

# You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.

clean:
	rm -f BlockGuard/*.gch
	rm -f BlockGuard/*.tmp
	rm -f BlockGuard/*.o
	rm -f *.out
	rm -f -r *.dSYM
	rm -f *.o
	rm -f BlockGuard_Test/*.gch
	rm -f BlockGuard_Test/*.tmp
	rm -f BlockGuard_Test/*.o
	clear
	clear

build:
	clang++ -std=c++14 ./BlockGuard/*.cpp BlockGuard/Logging/LogWritter.cpp --debug -D_GLIBCXX_DEBUG -o ./BlockGuard.out


test: AltBitPeer
	clang++ -std=c++14 ./BlockGuard_Test/*.cpp ./BlockGuard_Test/*.o --debug -D_GLIBCXX_DEBUG -o ./BlockGuard_Test.out

Logging:
	clang++ -std=c++14 BlockGuard/Logging/LogWritter.cpp -c --debug -D_GLIBCXX_DEBUG -o ./BlockGuard/LogWritter.o

AltBitPeer: 
	clang++ -std=c++14 BlockGuard/AltBitPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./BlockGuard_Test/AltBitPeer.o

BitcoinPeer: 
	clang++ -std=c++14 BlockGuard/BitcoinPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./BlockGuard_Test/BitcoinPeer.o

EthereumPeer: 
	clang++ -std=c++14 BlockGuard/EthereumPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./BlockGuard_Test/EthereumPeer.o

ExamplePeer: 
	clang++ -std=c++14 BlockGuard/ExamplePeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./BlockGuard_Test/ExamplePeer.o

KademliaPeer: 
	clang++ -std=c++14 BlockGuard/KademliaPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./BlockGuard_Test/KademliaPeer.o

LinearChordPeer: 
	clang++ -std=c++14 BlockGuard/LinearChordPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./BlockGuard_Test/LinearChordPeer.o

PBFTPeer: 
	clang++ -std=c++14 BlockGuard/PBFTPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./BlockGuard_Test/PBFTPeer.o

RaftPeer: 
	clang++ -std=c++14 BlockGuard/RaftPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./BlockGuard_Test/RaftPeer.o

StableDataLinkPeer: 
	clang++ -std=c++14 BlockGuard/StableDataLinkPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./BlockGuard_Test/StableDataLinkPeer.o
