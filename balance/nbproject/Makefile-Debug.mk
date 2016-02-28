#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/MotionSensor/I2Cdev/I2Cdev.o \
	${OBJECTDIR}/MotionSensor/balance.o \
	${OBJECTDIR}/MotionSensor/inv_mpu_lib/inv_mpu.o \
	${OBJECTDIR}/MotionSensor/inv_mpu_lib/inv_mpu_dmp_motion_driver.o \
	${OBJECTDIR}/MotionSensor/pi2golite.o \
	${OBJECTDIR}/MotionSensor/sensor.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=-lwiringPi -lpthread

# CC Compiler Flags
CCFLAGS=-lwiringPi -lpthread
CXXFLAGS=-lwiringPi -lpthread

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/balance

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/balance: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/balance ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/MotionSensor/I2Cdev/I2Cdev.o: MotionSensor/I2Cdev/I2Cdev.c 
	${MKDIR} -p ${OBJECTDIR}/MotionSensor/I2Cdev
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MotionSensor/I2Cdev/I2Cdev.o MotionSensor/I2Cdev/I2Cdev.c

${OBJECTDIR}/MotionSensor/balance.o: MotionSensor/balance.cpp 
	${MKDIR} -p ${OBJECTDIR}/MotionSensor
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MotionSensor/balance.o MotionSensor/balance.cpp

${OBJECTDIR}/MotionSensor/inv_mpu_lib/inv_mpu.o: MotionSensor/inv_mpu_lib/inv_mpu.c 
	${MKDIR} -p ${OBJECTDIR}/MotionSensor/inv_mpu_lib
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MotionSensor/inv_mpu_lib/inv_mpu.o MotionSensor/inv_mpu_lib/inv_mpu.c

${OBJECTDIR}/MotionSensor/inv_mpu_lib/inv_mpu_dmp_motion_driver.o: MotionSensor/inv_mpu_lib/inv_mpu_dmp_motion_driver.c 
	${MKDIR} -p ${OBJECTDIR}/MotionSensor/inv_mpu_lib
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MotionSensor/inv_mpu_lib/inv_mpu_dmp_motion_driver.o MotionSensor/inv_mpu_lib/inv_mpu_dmp_motion_driver.c

${OBJECTDIR}/MotionSensor/pi2golite.o: MotionSensor/pi2golite.cpp 
	${MKDIR} -p ${OBJECTDIR}/MotionSensor
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MotionSensor/pi2golite.o MotionSensor/pi2golite.cpp

${OBJECTDIR}/MotionSensor/sensor.o: MotionSensor/sensor.cpp 
	${MKDIR} -p ${OBJECTDIR}/MotionSensor
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MotionSensor/sensor.o MotionSensor/sensor.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/balance

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
