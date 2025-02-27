/*
 * RoDeO, a Robust Design Optimization Package
 *
 * Copyright (C) 2015-2021 Chair for Scientific Computing (SciComp), RPTU
 * Homepage: http://www.scicomp.uni-kl.de
 * Contact:  Prof. Nicolas R. Gauger (nicolas.gauger@scicomp.uni-kl.de) or Dr. Emre Özkaya (emre.oezkaya@scicomp.uni-kl.de)
 *
 * Lead developer: Emre Özkaya (SciComp, RPTU)
 *
 * This file is part of RoDeO
 *
 * RoDeO is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * RoDeO is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU
 * General Public License along with RoDeO.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Emre Özkaya, (SciComp, RPTU)
 *
 *
 *
 */

#include<gtest/gtest.h>
#include "output.hpp"
#include "test_defines.hpp"

#ifdef TEST_OUTPUT



TEST(testOutput, testConstructor){

	OutputDevice testObject;
	ASSERT_EQ(testObject.ifScreenDisplay, false);

}


TEST(testOutput, testprintMsgWithInt){

	OutputDevice testObject;
//	testObject.ifScreenDisplay = true;
	testObject.printMessage("hello world = ", 5);

}

TEST(testOutput, testprintMsg){

	OutputDevice testObject;
	testObject.printMessage("hello world");

}



#endif
