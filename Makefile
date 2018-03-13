# Brainf: Compiler of Brainf*** code
# Copyright (C) 2015 Filip Kocina
#
# This file is part of Brainf.
#
# Brainf is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Brainf is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Brainf.  If not, see <http://www.gnu.org/licenses/>.

.PHONY: install

BINDIR=/usr/local/bin
NAME=brainf
PROG=$(NAME)2c

$(PROG): $(PROG).c
	$(CC) -o $@ $<

install: $(PROG)
	cp -t $(BINDIR) $(NAME) $(PROG)
	chmod +x $(BINDIR)/$(NAME) $(BINDIR)/$(PROG)
