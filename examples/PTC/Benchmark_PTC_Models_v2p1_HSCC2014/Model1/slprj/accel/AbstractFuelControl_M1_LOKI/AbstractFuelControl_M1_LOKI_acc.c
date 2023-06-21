#include "AbstractFuelControl_M1_LOKI_acc.h"
#include "rtwtypes.h"
#include "mwmathutil.h"
#include <float.h>
#include "AbstractFuelControl_M1_LOKI_acc_private.h"
#include "multiword_types.h"
#include "simstruc_types.h"
#include <stdio.h>
#include "slexec_vm_simstruct_bridge.h"
#include "slexec_vm_zc_functions.h"
#include "slexec_vm_lookup_functions.h"
#include "slsv_diagnostic_codegen_c_api.h"
#include "simtarget/slSimTgtMdlrefSfcnBridge.h"
#include "simstruc.h"
#include "fixedpoint.h"
#define CodeFormat S-Function
#define AccDefine1 Accelerator_S-Function
#include "simtarget/slAccSfcnBridge.h"
#ifndef __RTW_UTFREE__  
extern void * utMalloc ( size_t ) ; extern void utFree ( void * ) ;
#endif
boolean_T AbstractFuelControl_M1_LOKI_acc_rt_TDelayUpdateTailOrGrowBuf (
int_T * bufSzPtr , int_T * tailPtr , int_T * headPtr , int_T * lastPtr ,
real_T tMinusDelay , real_T * * uBufPtr , boolean_T isfixedbuf , boolean_T
istransportdelay , int_T * maxNewBufSzPtr ) { int_T testIdx ; int_T tail = *
tailPtr ; int_T bufSz = * bufSzPtr ; real_T * tBuf = * uBufPtr + bufSz ;
real_T * xBuf = ( NULL ) ; int_T numBuffer = 2 ; if ( istransportdelay ) {
numBuffer = 3 ; xBuf = * uBufPtr + 2 * bufSz ; } testIdx = ( tail < ( bufSz -
1 ) ) ? ( tail + 1 ) : 0 ; if ( ( tMinusDelay <= tBuf [ testIdx ] ) && !
isfixedbuf ) { int_T j ; real_T * tempT ; real_T * tempU ; real_T * tempX = (
NULL ) ; real_T * uBuf = * uBufPtr ; int_T newBufSz = bufSz + 1024 ; if (
newBufSz > * maxNewBufSzPtr ) { * maxNewBufSzPtr = newBufSz ; } tempU = (
real_T * ) utMalloc ( numBuffer * newBufSz * sizeof ( real_T ) ) ; if ( tempU
== ( NULL ) ) { return ( false ) ; } tempT = tempU + newBufSz ; if (
istransportdelay ) tempX = tempT + newBufSz ; for ( j = tail ; j < bufSz ; j
++ ) { tempT [ j - tail ] = tBuf [ j ] ; tempU [ j - tail ] = uBuf [ j ] ; if
( istransportdelay ) tempX [ j - tail ] = xBuf [ j ] ; } for ( j = 0 ; j <
tail ; j ++ ) { tempT [ j + bufSz - tail ] = tBuf [ j ] ; tempU [ j + bufSz -
tail ] = uBuf [ j ] ; if ( istransportdelay ) tempX [ j + bufSz - tail ] =
xBuf [ j ] ; } if ( * lastPtr > tail ) { * lastPtr -= tail ; } else { *
lastPtr += ( bufSz - tail ) ; } * tailPtr = 0 ; * headPtr = bufSz ; utFree (
uBuf ) ; * bufSzPtr = newBufSz ; * uBufPtr = tempU ; } else { * tailPtr =
testIdx ; } return ( true ) ; } real_T
AbstractFuelControl_M1_LOKI_acc_rt_VTDelayfindtDInterpolate ( real_T x ,
real_T * uBuf , int_T bufSz , int_T head , int_T tail , int_T * pLast ,
real_T t , real_T tStart , boolean_T discrete , boolean_T
minorStepAndTAtLastMajorOutput , real_T initOutput , real_T * appliedDelay )
{ int_T n , k ; real_T f ; int_T kp1 ; real_T tminustD , tL , tR , uD , uL ,
uR , fU ; real_T * tBuf = uBuf + bufSz ; real_T * xBuf = uBuf + 2 * bufSz ;
if ( minorStepAndTAtLastMajorOutput ) { if ( * pLast == head ) { * pLast = (
* pLast == 0 ) ? bufSz - 1 : * pLast - 1 ; } head = ( head == 0 ) ? bufSz - 1
: head - 1 ; } if ( x <= 1 ) { return initOutput ; } k = * pLast ; n = 0 ;
for ( ; ; ) { n ++ ; if ( n > bufSz ) break ; if ( x - xBuf [ k ] > 1.0 ) {
if ( k == head ) { int_T km1 ; f = ( x - 1.0 - xBuf [ k ] ) / ( x - xBuf [ k
] ) ; tminustD = ( 1.0 - f ) * tBuf [ k ] + f * t ; km1 = k - 1 ; if ( km1 <
0 ) km1 = bufSz - 1 ; tL = tBuf [ km1 ] ; tR = tBuf [ k ] ; uL = uBuf [ km1 ]
; uR = uBuf [ k ] ; break ; } kp1 = k + 1 ; if ( kp1 == bufSz ) kp1 = 0 ; if
( x - xBuf [ kp1 ] <= 1.0 ) { f = ( x - 1.0 - xBuf [ k ] ) / ( xBuf [ kp1 ] -
xBuf [ k ] ) ; tL = tBuf [ k ] ; tR = tBuf [ kp1 ] ; uL = uBuf [ k ] ; uR =
uBuf [ kp1 ] ; tminustD = ( 1.0 - f ) * tL + f * tR ; break ; } k = kp1 ; }
else { if ( k == tail ) { f = ( x - 1.0 ) / xBuf [ k ] ; if ( discrete ) {
return ( uBuf [ tail ] ) ; } kp1 = k + 1 ; if ( kp1 == bufSz ) kp1 = 0 ;
tminustD = ( 1 - f ) * tStart + f * tBuf [ k ] ; tL = tBuf [ k ] ; tR = tBuf
[ kp1 ] ; uL = uBuf [ k ] ; uR = uBuf [ kp1 ] ; break ; } k = k - 1 ; if ( k
< 0 ) k = bufSz - 1 ; } } * pLast = k ; if ( tR == tL ) { fU = 1.0 ; } else {
fU = ( tminustD - tL ) / ( tR - tL ) ; } if ( discrete ) { uD = ( fU > ( 1.0
- fU ) ) ? uR : uL ; } else { uD = ( 1.0 - fU ) * uL + fU * uR ; } *
appliedDelay = t - tminustD ; return uD ; } real_T look2_binlxpw ( real_T u0
, real_T u1 , const real_T bp0 [ ] , const real_T bp1 [ ] , const real_T
table [ ] , const uint32_T maxIndex [ ] , uint32_T stride ) { real_T
fractions [ 2 ] ; real_T frac ; real_T yL_0d0 ; real_T yL_0d1 ; uint32_T
bpIndices [ 2 ] ; uint32_T bpIdx ; uint32_T iLeft ; uint32_T iRght ; if ( u0
<= bp0 [ 0U ] ) { iLeft = 0U ; frac = ( u0 - bp0 [ 0U ] ) / ( bp0 [ 1U ] -
bp0 [ 0U ] ) ; } else if ( u0 < bp0 [ maxIndex [ 0U ] ] ) { bpIdx = maxIndex
[ 0U ] >> 1U ; iLeft = 0U ; iRght = maxIndex [ 0U ] ; while ( iRght - iLeft >
1U ) { if ( u0 < bp0 [ bpIdx ] ) { iRght = bpIdx ; } else { iLeft = bpIdx ; }
bpIdx = ( iRght + iLeft ) >> 1U ; } frac = ( u0 - bp0 [ iLeft ] ) / ( bp0 [
iLeft + 1U ] - bp0 [ iLeft ] ) ; } else { iLeft = maxIndex [ 0U ] - 1U ; frac
= ( u0 - bp0 [ maxIndex [ 0U ] - 1U ] ) / ( bp0 [ maxIndex [ 0U ] ] - bp0 [
maxIndex [ 0U ] - 1U ] ) ; } fractions [ 0U ] = frac ; bpIndices [ 0U ] =
iLeft ; if ( u1 <= bp1 [ 0U ] ) { iLeft = 0U ; frac = ( u1 - bp1 [ 0U ] ) / (
bp1 [ 1U ] - bp1 [ 0U ] ) ; } else if ( u1 < bp1 [ maxIndex [ 1U ] ] ) {
bpIdx = maxIndex [ 1U ] >> 1U ; iLeft = 0U ; iRght = maxIndex [ 1U ] ; while
( iRght - iLeft > 1U ) { if ( u1 < bp1 [ bpIdx ] ) { iRght = bpIdx ; } else {
iLeft = bpIdx ; } bpIdx = ( iRght + iLeft ) >> 1U ; } frac = ( u1 - bp1 [
iLeft ] ) / ( bp1 [ iLeft + 1U ] - bp1 [ iLeft ] ) ; } else { iLeft =
maxIndex [ 1U ] - 1U ; frac = ( u1 - bp1 [ maxIndex [ 1U ] - 1U ] ) / ( bp1 [
maxIndex [ 1U ] ] - bp1 [ maxIndex [ 1U ] - 1U ] ) ; } bpIdx = iLeft * stride
+ bpIndices [ 0U ] ; yL_0d0 = table [ bpIdx ] ; yL_0d0 += ( table [ bpIdx +
1U ] - yL_0d0 ) * fractions [ 0U ] ; bpIdx += stride ; yL_0d1 = table [ bpIdx
] ; return ( ( ( table [ bpIdx + 1U ] - yL_0d1 ) * fractions [ 0U ] + yL_0d1
) - yL_0d0 ) * frac + yL_0d0 ; } void rt_ssGetBlockPath ( SimStruct * S ,
int_T sysIdx , int_T blkIdx , char_T * * path ) { _ssGetBlockPath ( S ,
sysIdx , blkIdx , path ) ; } void rt_ssSet_slErrMsg ( void * S , void * diag
) { SimStruct * castedS = ( SimStruct * ) S ; if ( !
_ssIsErrorStatusAslErrMsg ( castedS ) ) { _ssSet_slErrMsg ( castedS , diag )
; } else { _ssDiscardDiagnostic ( castedS , diag ) ; } } void
rt_ssReportDiagnosticAsWarning ( void * S , void * diag ) {
_ssReportDiagnosticAsWarning ( ( SimStruct * ) S , diag ) ; } void
rt_ssReportDiagnosticAsInfo ( void * S , void * diag ) {
_ssReportDiagnosticAsInfo ( ( SimStruct * ) S , diag ) ; } static void
mdlOutputs ( SimStruct * S , int_T tid ) { real_T B_15_46_0 ; boolean_T
B_13_4_0 ; B_AbstractFuelControl_M1_LOKI_T * _rtB ;
DW_AbstractFuelControl_M1_LOKI_T * _rtDW ; P_AbstractFuelControl_M1_LOKI_T *
_rtP ; PrevZCX_AbstractFuelControl_M1_LOKI_T * _rtZCE ;
X_AbstractFuelControl_M1_LOKI_T * _rtX ; real_T ratio ; real_T rtb_B_13_0_0 ;
real_T rtb_B_15_0_0 ; real_T rtb_B_15_10_0 ; real_T rtb_B_15_14_0 ; real_T
rtb_B_15_19_0 ; real_T rtb_B_15_2_0 ; real_T taskTimeV ; int32_T isHit ;
real32_T rtb_B_15_11_0 ; real32_T rtb_B_15_12_0 ; uint32_T numCycles ;
ZCEventType zcEvent ; _rtDW = ( ( DW_AbstractFuelControl_M1_LOKI_T * )
ssGetRootDWork ( S ) ) ; _rtZCE = ( ( PrevZCX_AbstractFuelControl_M1_LOKI_T *
) _ssGetPrevZCSigState ( S ) ) ; _rtX = ( ( X_AbstractFuelControl_M1_LOKI_T *
) ssGetContStates ( S ) ) ; _rtP = ( ( P_AbstractFuelControl_M1_LOKI_T * )
ssGetModelRtp ( S ) ) ; _rtB = ( ( B_AbstractFuelControl_M1_LOKI_T * )
_ssGetModelBlockIO ( S ) ) ; isHit = ssIsSampleHit ( S , 1 , 0 ) ; if ( isHit
!= 0 ) { _rtB -> B_20_2_0 [ 0 ] = _rtP -> P_55 [ 0 ] * _rtDW ->
Memory_PreviousInput [ 0 ] + _rtB -> B_20_8_0 [ 0 ] ; _rtB -> B_20_2_0 [ 1 ]
= _rtP -> P_55 [ 1 ] * _rtDW -> Memory_PreviousInput [ 1 ] + _rtB -> B_20_8_0
[ 1 ] ; } rtb_B_15_0_0 = _rtX -> Integrator_CSTATE ; _rtB -> B_15_1_0 = _rtP
-> P_8 * _rtX -> Integrator_CSTATE ; _rtB -> B_15_3_0 = _rtP -> P_10 * _rtX
-> Throttledelay_CSTATE + _rtB -> B_15_2_0 ; if ( ssIsModeUpdateTimeStep ( S
) ) { _rtDW -> theta090_MODE = _rtB -> B_15_3_0 >= _rtP -> P_11 ? 1 : _rtB ->
B_15_3_0 > _rtP -> P_12 ? 0 : - 1 ; } _rtB -> B_15_4_0 = _rtDW ->
theta090_MODE == 1 ? _rtP -> P_11 : _rtDW -> theta090_MODE == - 1 ? _rtP ->
P_12 : _rtB -> B_15_3_0 ; ssCallAccelRunBlock ( S , 15 , 5 ,
SS_CALL_MDL_OUTPUTS ) ; isHit = ssIsSampleHit ( S , 1 , 0 ) ; if ( isHit != 0
) { if ( _rtB -> B_20_2_0 [ 1 ] > _rtP -> P_13 ) { rtb_B_15_19_0 = _rtP ->
P_13 ; } else if ( _rtB -> B_20_2_0 [ 1 ] < _rtP -> P_14 ) { rtb_B_15_19_0 =
_rtP -> P_14 ; } else { rtb_B_15_19_0 = _rtB -> B_20_2_0 [ 1 ] ; } _rtB ->
B_15_7_0 = _rtP -> P_15 * rtb_B_15_19_0 ; _rtDW ->
AFSensorFaultInjection_MODE = ( ssGetTaskTime ( S , 1 ) >= _rtP -> P_16 ) ;
if ( _rtDW -> AFSensorFaultInjection_MODE == 1 ) { _rtB -> B_15_8_0 = _rtP ->
P_18 ; } else { _rtB -> B_15_8_0 = _rtP -> P_17 ; } } if ( _rtB -> B_15_8_0
>= _rtP -> P_19 ) { rtb_B_15_19_0 = _rtB -> B_15_3_0_p ; } else {
rtb_B_15_19_0 = _rtB -> B_15_1_0 ; } rtb_B_15_10_0 = _rtP -> P_20 *
rtb_B_15_19_0 ; isHit = ssIsSampleHit ( S , 1 , 0 ) ; if ( isHit != 0 ) {
rtb_B_15_11_0 = ( real32_T ) _rtB -> B_15_7_0 ; } rtb_B_15_12_0 = ( real32_T
) _rtB -> B_15_4_0 ; rtb_B_15_14_0 = _rtX -> p00543bar_CSTATE ; _rtB ->
B_15_15_0 = _rtX -> p00543bar_CSTATE / _rtB -> B_15_1_0_b ; _rtB -> B_15_16_0
= 1.0 / _rtX -> p00543bar_CSTATE * _rtB -> B_15_1_0_b ; if (
ssIsModeUpdateTimeStep ( S ) ) { _rtB -> B_15_17_0 = _rtB -> B_15_15_0 ;
_rtDW -> MinMax_MODE_c = 0 ; if ( ( _rtB -> B_15_15_0 != _rtB -> B_15_15_0 )
|| ( _rtB -> B_15_16_0 < _rtB -> B_15_15_0 ) ) { _rtB -> B_15_17_0 = _rtB ->
B_15_16_0 ; _rtDW -> MinMax_MODE_c = 1 ; } _rtDW -> Switch_Mode = ( _rtB ->
B_15_17_0 >= _rtP -> P_22 ) ; } else { _rtB -> B_15_17_0 = _rtB -> B_15_15_0
; if ( _rtDW -> MinMax_MODE_c == 1 ) { _rtB -> B_15_17_0 = _rtB -> B_15_16_0
; } } if ( _rtDW -> Switch_Mode ) { rtb_B_15_19_0 = _rtB -> B_15_17_0 - _rtB
-> B_15_17_0 * _rtB -> B_15_17_0 ; if ( rtb_B_15_19_0 < 0.0 ) { rtb_B_15_19_0
= - muDoubleScalarSqrt ( - rtb_B_15_19_0 ) ; } else { rtb_B_15_19_0 =
muDoubleScalarSqrt ( rtb_B_15_19_0 ) ; } _rtB -> B_14_0_0 = 2.0 *
rtb_B_15_19_0 ; rtb_B_15_19_0 = _rtB -> B_14_0_0 ; } else { rtb_B_15_19_0 =
_rtB -> B_15_4_0_c ; } _rtB -> B_15_20_0 = _rtB -> B_15_1_0_b - _rtX ->
p00543bar_CSTATE ; isHit = ssIsSampleHit ( S , 1 , 0 ) ; if ( isHit != 0 ) {
if ( _rtB -> B_15_20_0 > 0.0 ) { _rtDW -> flowdirection_MODE = 1 ; } else if
( _rtB -> B_15_20_0 < 0.0 ) { _rtDW -> flowdirection_MODE = - 1 ; } else {
_rtDW -> flowdirection_MODE = 0 ; } _rtB -> B_15_21_0 = _rtDW ->
flowdirection_MODE ; } rtb_B_15_2_0 = ( ( ( 2.821 - 0.05231 * _rtB ->
B_15_4_0 ) + 0.10299 * _rtB -> B_15_4_0 * _rtB -> B_15_4_0 ) - 0.00063 * _rtB
-> B_15_4_0 * _rtB -> B_15_4_0 * _rtB -> B_15_4_0 ) * rtb_B_15_19_0 * _rtB ->
B_15_21_0 ; rtb_B_15_19_0 = _rtP -> P_23 * rtb_B_15_2_0 ; isHit =
ssIsSampleHit ( S , 1 , 0 ) ; if ( isHit != 0 ) { _rtDW -> Pwon_MODE = (
ssGetTaskTime ( S , 1 ) >= _rtP -> P_0 ) ; if ( _rtDW -> Pwon_MODE == 1 ) {
rtb_B_13_0_0 = _rtP -> P_2 ; } else { rtb_B_13_0_0 = _rtP -> P_1 ; } } isHit
= ssIsSampleHit ( S , 3 , 0 ) ; if ( isHit != 0 ) { taskTimeV = ssGetTaskTime
( S , 3 ) ; if ( ssGetTNextWasAdjusted ( S , 3 ) != 0 ) { _rtDW -> nextTime =
_ssGetVarNextHitTime ( S , 0 ) ; } if ( _rtDW -> justEnabled != 0 ) { _rtDW
-> justEnabled = 0 ; if ( taskTimeV >= _rtP -> P_6 ) { ratio = ( taskTimeV -
_rtP -> P_6 ) / _rtP -> P_4 ; numCycles = ( uint32_T ) muDoubleScalarFloor (
ratio ) ; if ( muDoubleScalarAbs ( ( real_T ) ( numCycles + 1U ) - ratio ) <
DBL_EPSILON * ratio ) { numCycles ++ ; } _rtDW -> numCompleteCycles =
numCycles ; ratio = ( ( real_T ) numCycles * _rtP -> P_4 + _rtP -> P_6 ) +
_rtP -> P_5 * _rtP -> P_4 / 100.0 ; if ( taskTimeV < ratio ) { _rtDW ->
currentValue = 1 ; _rtDW -> nextTime = ratio ; } else { _rtDW -> currentValue
= 0 ; _rtDW -> nextTime = ( real_T ) ( numCycles + 1U ) * _rtP -> P_4 + _rtP
-> P_6 ; } } else { _rtDW -> numCompleteCycles = _rtP -> P_6 != 0.0 ? - 1 : 0
; _rtDW -> currentValue = 0 ; _rtDW -> nextTime = _rtP -> P_6 ; } } else if (
_rtDW -> nextTime <= taskTimeV ) { if ( _rtDW -> currentValue == 1 ) { _rtDW
-> currentValue = 0 ; _rtDW -> nextTime = ( real_T ) ( _rtDW ->
numCompleteCycles + 1LL ) * _rtP -> P_4 + _rtP -> P_6 ; } else { _rtDW ->
numCompleteCycles ++ ; _rtDW -> currentValue = 1 ; _rtDW -> nextTime = ( _rtP
-> P_5 * _rtP -> P_4 * 0.01 + ( real_T ) _rtDW -> numCompleteCycles * _rtP ->
P_4 ) + _rtP -> P_6 ; } } _ssSetVarNextHitTime ( S , 0 , _rtDW -> nextTime )
; if ( _rtDW -> currentValue == 1 ) { _rtB -> B_13_1_0 = _rtP -> P_3 ; } else
{ _rtB -> B_13_1_0 = 0.0 ; } } isHit = ssIsSampleHit ( S , 1 , 0 ) ; if (
isHit != 0 ) { _rtDW -> engine_speed = rtb_B_15_11_0 ; vm_WriteLocalDSMNoIdx
( S , _rtDW -> dsmIdx_f , ( char_T * )
 "AbstractFuelControl_M1_LOKI/Model 1/AF_Controller/fuel_controller/DataStoreWrite"
, 0 ) ; } _rtDW -> throttle_angle = rtb_B_15_12_0 ; vm_WriteLocalDSMNoIdx ( S
, _rtDW -> dsmIdx_m , ( char_T * )
 "AbstractFuelControl_M1_LOKI/Model 1/AF_Controller/fuel_controller/DataStoreWrite3"
, 0 ) ; _rtDW -> throttle_flow = ( real32_T ) rtb_B_15_19_0 ;
vm_WriteLocalDSMNoIdx ( S , _rtDW -> dsmIdx_h , ( char_T * )
 "AbstractFuelControl_M1_LOKI/Model 1/AF_Controller/fuel_controller/DataStoreWrite1"
, 0 ) ; _rtDW -> airbyfuel_meas = ( real32_T ) rtb_B_15_10_0 ;
vm_WriteLocalDSMNoIdx ( S , _rtDW -> dsmIdx_ha , ( char_T * )
 "AbstractFuelControl_M1_LOKI/Model 1/AF_Controller/fuel_controller/DataStoreWrite2"
, 0 ) ; isHit = ssIsSampleHit ( S , 1 , 0 ) ; if ( ( isHit != 0 ) &&
ssIsModeUpdateTimeStep ( S ) ) { zcEvent = rt_ZCFcn ( RISING_ZERO_CROSSING ,
& _rtZCE -> fuel_controller_pwon_Trig_ZCE , rtb_B_13_0_0 ) ; if ( zcEvent !=
NO_ZCEVENT ) { _rtDW -> controller_mode = ( _rtP -> P_98 != 0.0F ) ;
vm_WriteLocalDSMNoIdx ( S , _rtDW -> dsmIdx_l , ( char_T * )
 "AbstractFuelControl_M1_LOKI/Model 1/AF_Controller/fuel_controller/fuel_controller_pwon/DataStoreWrite1"
, 0 ) ; _rtDW -> commanded_fuel = _rtP -> P_99 ; vm_WriteLocalDSMNoIdx ( S ,
_rtDW -> dsmIdx , ( char_T * )
 "AbstractFuelControl_M1_LOKI/Model 1/AF_Controller/fuel_controller/fuel_controller_pwon/DataStoreWrite"
, 0 ) ; if ( ssIsMajorTimeStep ( S ) != 0 ) {
ssSetBlockStateForSolverChangedAtMajorStep ( S ) ;
ssSetContTimeOutputInconsistentWithStateAtMajorStep ( S ) ; } _rtDW ->
airbyfuel_ref = _rtP -> P_100 ; vm_WriteLocalDSMNoIdx ( S , _rtDW -> dsmIdx_e
, ( char_T * )
 "AbstractFuelControl_M1_LOKI/Model 1/AF_Controller/fuel_controller/fuel_controller_pwon/DataStoreWrite2"
, 0 ) ; if ( ssIsMajorTimeStep ( S ) != 0 ) {
ssSetBlockStateForSolverChangedAtMajorStep ( S ) ;
ssSetContTimeOutputInconsistentWithStateAtMajorStep ( S ) ; } _rtDW ->
fuel_controller_pwon_SubsysRanBC = 4 ; } zcEvent = rt_ZCFcn (
RISING_ZERO_CROSSING , & _rtZCE -> fuel_controller_mode_10ms_Trig_ZCE , _rtB
-> B_13_1_0 ) ; if ( zcEvent != NO_ZCEVENT ) { vm_ReadLocalDSMNoIdx ( S ,
_rtDW -> dsmIdx_ha , ( char_T * )
 "AbstractFuelControl_M1_LOKI/Model 1/AF_Controller/fuel_controller/fuel_controller_mode_10ms/DataStoreRead2"
, 0 ) ; isHit = ssIsSampleHit ( S , 1 , 0 ) ; if ( isHit != 0 ) { _rtB ->
B_9_2_0 = ( ( _rtDW -> airbyfuel_meas <= _rtB -> B_9_0_0 ) || _rtDW ->
UnitDelay_DSTATE_g ) ; } _rtB -> B_7_2_0 = _rtDW -> UnitDelay2_DSTATE + _rtP
-> P_93 ; _rtB -> B_7_6_0 = ( ( _rtB -> B_7_2_0 >= _rtP -> P_94 ) || _rtDW ->
UnitDelay1_DSTATE_e ) ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx_m , (
char_T * )
 "AbstractFuelControl_M1_LOKI/Model 1/AF_Controller/fuel_controller/fuel_controller_mode_10ms/DataStoreRead4"
, 0 ) ; isHit = ssIsSampleHit ( S , 1 , 0 ) ; if ( isHit != 0 ) { if ( _rtDW
-> UnitDelay1_DSTATE_a ) { rtb_B_15_11_0 = _rtB -> B_8_0_0 ; } else {
rtb_B_15_11_0 = _rtB -> B_8_2_0 ; } _rtB -> B_8_2_0_f = ( _rtDW ->
throttle_angle >= rtb_B_15_11_0 ) ; } _rtDW -> controller_mode = ( _rtB ->
B_9_2_0 || ( ! _rtB -> B_7_6_0 ) || _rtB -> B_8_2_0_f ) ;
vm_WriteLocalDSMNoIdx ( S , _rtDW -> dsmIdx_l , ( char_T * )
 "AbstractFuelControl_M1_LOKI/Model 1/AF_Controller/fuel_controller/fuel_controller_mode_10ms/DataStoreWrite"
, 0 ) ; if ( _rtB -> B_7_6_0 && _rtB -> B_8_2_0_f ) { _rtDW -> airbyfuel_ref
= _rtP -> P_90 ; } else { _rtDW -> airbyfuel_ref = _rtP -> P_91 ; }
vm_WriteLocalDSMNoIdx ( S , _rtDW -> dsmIdx_e , ( char_T * )
 "AbstractFuelControl_M1_LOKI/Model 1/AF_Controller/fuel_controller/fuel_controller_mode_10ms/DataStoreWrite1"
, 0 ) ; if ( ssIsMajorTimeStep ( S ) != 0 ) {
ssSetBlockStateForSolverChangedAtMajorStep ( S ) ;
ssSetContTimeOutputInconsistentWithStateAtMajorStep ( S ) ; } isHit =
ssIsSampleHit ( S , 1 , 0 ) ; if ( isHit != 0 ) { _rtDW -> UnitDelay_DSTATE_g
= _rtB -> B_9_2_0 ; } _rtDW -> UnitDelay2_DSTATE = _rtB -> B_7_2_0 ; _rtDW ->
UnitDelay1_DSTATE_e = _rtB -> B_7_6_0 ; isHit = ssIsSampleHit ( S , 1 , 0 ) ;
if ( isHit != 0 ) { _rtDW -> UnitDelay1_DSTATE_a = _rtB -> B_8_2_0_f ; }
_rtDW -> fuel_controller_mode_10ms_SubsysRanBC = 4 ; } zcEvent = rt_ZCFcn (
RISING_ZERO_CROSSING , & _rtZCE -> fuel_controller_10ms_Trig_ZCE , _rtB ->
B_13_1_0 ) ; if ( zcEvent != NO_ZCEVENT ) { vm_ReadLocalDSMNoIdx ( S , _rtDW
-> dsmIdx_h , ( char_T * )
 "AbstractFuelControl_M1_LOKI/Model 1/AF_Controller/fuel_controller/fuel_controller_10ms/DataStoreRead"
, 0 ) ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx_f , ( char_T * )
 "AbstractFuelControl_M1_LOKI/Model 1/AF_Controller/fuel_controller/fuel_controller_10ms/DataStoreRead1"
, 0 ) ; rtb_B_15_11_0 = ( ( _rtDW -> UnitDelay1_DSTATE_d * _rtDW ->
engine_speed * _rtP -> P_77 + _rtP -> P_76 ) + _rtDW -> UnitDelay1_DSTATE_d *
_rtDW -> UnitDelay1_DSTATE_d * _rtDW -> engine_speed * _rtP -> P_78 ) + _rtDW
-> engine_speed * _rtDW -> engine_speed * _rtDW -> UnitDelay1_DSTATE_d * _rtP
-> P_79 ; _rtB -> B_1_13_0 = ( _rtDW -> throttle_flow - rtb_B_15_11_0 ) *
_rtP -> P_81 * _rtP -> P_75 + _rtDW -> UnitDelay1_DSTATE_d ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx_e , ( char_T * )
 "AbstractFuelControl_M1_LOKI/Model 1/AF_Controller/fuel_controller/fuel_controller_10ms/DataStoreRead4"
, 0 ) ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx_l , ( char_T * )
 "AbstractFuelControl_M1_LOKI/Model 1/AF_Controller/fuel_controller/fuel_controller_10ms/DataStoreRead3"
, 0 ) ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx_ha , ( char_T * )
 "AbstractFuelControl_M1_LOKI/Model 1/AF_Controller/fuel_controller/fuel_controller_10ms/DataStoreRead2"
, 0 ) ; _rtB -> B_4_8_0 = ! _rtDW -> controller_mode ; if ( _rtB -> B_4_8_0 )
{ rtb_B_15_12_0 = _rtDW -> airbyfuel_meas - _rtDW -> airbyfuel_ref ; _rtB ->
B_2_6_0 = _rtP -> P_84 * rtb_B_15_12_0 * _rtP -> P_82 + _rtDW ->
UnitDelay1_DSTATE ; _rtB -> B_2_7_0 = _rtP -> P_83 * rtb_B_15_12_0 + _rtB ->
B_2_6_0 ; _rtDW -> feedback_PI_controller_SubsysRanBC = 4 ; } if ( _rtDW ->
controller_mode ) { rtb_B_15_12_0 = _rtB -> B_4_1_0 ; } else { rtb_B_15_12_0
= _rtB -> B_4_0_0 + _rtB -> B_2_7_0 ; if ( rtb_B_15_12_0 > _rtP -> P_73 ) {
rtb_B_15_12_0 = _rtP -> P_73 ; } else if ( rtb_B_15_12_0 < _rtP -> P_74 ) {
rtb_B_15_12_0 = _rtP -> P_74 ; } } rtb_B_15_11_0 = rtb_B_15_11_0 / _rtDW ->
airbyfuel_ref * rtb_B_15_12_0 ; if ( rtb_B_15_11_0 > _rtP -> P_86 ) { _rtDW
-> commanded_fuel = _rtP -> P_86 ; } else if ( rtb_B_15_11_0 < _rtP -> P_87 )
{ _rtDW -> commanded_fuel = _rtP -> P_87 ; } else { _rtDW -> commanded_fuel =
rtb_B_15_11_0 ; } vm_WriteLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
 "AbstractFuelControl_M1_LOKI/Model 1/AF_Controller/fuel_controller/fuel_controller_10ms/DataStoreWrite"
, 0 ) ; if ( ssIsMajorTimeStep ( S ) != 0 ) {
ssSetBlockStateForSolverChangedAtMajorStep ( S ) ;
ssSetContTimeOutputInconsistentWithStateAtMajorStep ( S ) ; } _rtDW ->
UnitDelay1_DSTATE_d = _rtB -> B_1_13_0 ; if ( _rtB -> B_4_8_0 ) { _rtDW ->
UnitDelay1_DSTATE = _rtB -> B_2_6_0 ; } _rtDW ->
fuel_controller_10ms_SubsysRanBC = 4 ; } } vm_ReadLocalDSMNoIdx ( S , _rtDW
-> dsmIdx , ( char_T * )
"AbstractFuelControl_M1_LOKI/Model 1/AF_Controller/DataStoreRead" , 0 ) ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx_l , ( char_T * )
"AbstractFuelControl_M1_LOKI/Model 1/AF_Controller/DataStoreRead1" , 0 ) ;
B_13_4_0 = _rtDW -> controller_mode ; vm_ReadLocalDSMNoIdx ( S , _rtDW ->
dsmIdx_e , ( char_T * )
"AbstractFuelControl_M1_LOKI/Model 1/AF_Controller/DataStoreRead2" , 0 ) ;
_rtB -> B_13_5_0 = _rtDW -> airbyfuel_ref ; rtb_B_13_0_0 = _rtX ->
Integrator_CSTATE_h ; _rtB -> B_15_29_0 = ( _rtX -> Integrator_CSTATE_h -
rtb_B_15_0_0 ) * _rtP -> P_25 ; rtb_B_15_0_0 = ( ( ( 0.08979 * rtb_B_15_14_0
* _rtB -> B_15_7_0 - 0.366 ) - 0.0337 * _rtB -> B_15_7_0 * rtb_B_15_14_0 *
rtb_B_15_14_0 ) + 0.0001 * rtb_B_15_14_0 * _rtB -> B_15_7_0 * _rtB ->
B_15_7_0 ) * _rtP -> P_107 ; isHit = ssIsSampleHit ( S , 1 , 0 ) ; if ( isHit
!= 0 ) { _rtB -> B_15_32_0 = _rtP -> P_26 * _rtB -> B_15_7_0 ; }
rtb_B_15_14_0 = rtb_B_15_0_0 / _rtB -> B_15_7_0 * _rtP -> P_27 ;
rtb_B_15_10_0 = _rtP -> P_31 * look2_binlxpw ( _rtB -> B_15_32_0 ,
rtb_B_15_14_0 , _rtP -> P_29 , _rtP -> P_30 , _rtP -> P_28 , _rtP -> P_108 ,
5U ) ; rtb_B_15_19_0 = _rtP -> P_32 * _rtDW -> commanded_fuel ; taskTimeV =
_rtX -> Integrator_CSTATE_c / ( _rtP -> P_37 * look2_binlxpw ( _rtB ->
B_15_32_0 , rtb_B_15_14_0 , _rtP -> P_35 , _rtP -> P_36 , _rtP -> P_34 , _rtP
-> P_109 , 5U ) ) ; _rtB -> B_15_45_0 = rtb_B_15_0_0 / ( rtb_B_15_10_0 *
rtb_B_15_19_0 + taskTimeV ) ; { real_T * * uBuffer = ( real_T * * ) & _rtDW
-> fuelsystemtransportdelay_PWORK . TUbufferPtrs [ 0 ] ; real_T simTime =
ssGetT ( S ) ; real_T appliedDelay ; B_15_46_0 =
AbstractFuelControl_M1_LOKI_acc_rt_VTDelayfindtDInterpolate ( ( (
X_AbstractFuelControl_M1_LOKI_T * ) ssGetContStates ( S ) ) ->
fuelsystemtransportdelay_CSTATE , * uBuffer , _rtDW ->
fuelsystemtransportdelay_IWORK . CircularBufSize , _rtDW ->
fuelsystemtransportdelay_IWORK . Head , _rtDW ->
fuelsystemtransportdelay_IWORK . Tail , & _rtDW ->
fuelsystemtransportdelay_IWORK . Last , simTime , 0.0 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ( * uBuffer + _rtDW ->
fuelsystemtransportdelay_IWORK . CircularBufSize ) [ _rtDW ->
fuelsystemtransportdelay_IWORK . Head ] == ssGetT ( S ) ) ) , _rtP -> P_39 ,
& appliedDelay ) ; } _rtB -> B_15_48_0 = ( B_15_46_0 - rtb_B_13_0_0 ) * _rtP
-> P_40 ; isHit = ssIsSampleHit ( S , 1 , 0 ) ; if ( isHit != 0 ) { _rtB ->
B_15_49_0 = _rtP -> P_41 * _rtB -> B_15_7_0 ; } _rtB -> B_15_50_0 =
look2_binlxpw ( _rtB -> B_15_49_0 , rtb_B_15_14_0 , _rtP -> P_43 , _rtP ->
P_44 , _rtP -> P_42 , _rtP -> P_110 , 5U ) ; _rtB -> B_15_52_0 = (
rtb_B_15_2_0 - rtb_B_15_0_0 ) * _rtP -> P_45 ; _rtB -> B_15_56_0 = ( _rtP ->
P_46 * rtb_B_15_10_0 + _rtB -> B_15_5_0 ) * rtb_B_15_19_0 - taskTimeV ; isHit
= ssIsSampleHit ( S , 1 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_20_4_0 = _rtDW
-> Memory_PreviousInput_e ; _rtB -> B_20_5_0 = _rtP -> P_57 * _rtB ->
B_20_4_0 ; } _rtB -> B_20_6_0 = ssGetT ( S ) ; if ( ssIsModeUpdateTimeStep (
S ) ) { _rtDW -> Switch2_Mode = ( _rtB -> B_20_6_0 >= _rtP -> P_58 ) ; } if (
_rtDW -> Switch2_Mode ) { _rtB -> B_20_7_0 = _rtB -> B_20_5_0 ; } else { _rtB
-> B_20_7_0 = _rtB -> B_20_7_0_c ; } if ( ssIsModeUpdateTimeStep ( S ) ) {
_rtDW -> Switch3_Mode = ( _rtB -> B_20_7_0 >= _rtP -> P_59 ) ; } if ( _rtDW
-> Switch3_Mode ) { _rtB -> B_19_0_0 = _rtP -> P_53 * _rtB -> B_20_7_0 ; _rtB
-> B_20_9_0 = _rtB -> B_19_0_0 ; } else { _rtB -> B_20_9_0 = _rtB -> B_20_7_0
; } ssCallAccelRunBlock ( S , 20 , 10 , SS_CALL_MDL_OUTPUTS ) ; isHit =
ssIsSampleHit ( S , 1 , 0 ) ; if ( isHit != 0 ) { if ( ssIsModeUpdateTimeStep
( S ) ) { _rtDW -> Compare_Mode = ( _rtB -> B_20_6_0 >= _rtB -> B_20_0_0 ) ;
} _rtB -> B_20_11_0 = _rtDW -> Compare_Mode ; } _rtB -> B_20_15_0 [ 0 ] =
_rtP -> P_60 [ 0 ] * _rtB -> B_15_1_0 + _rtB -> B_20_1_0 [ 0 ] ; _rtB ->
B_20_15_0 [ 1 ] = _rtP -> P_60 [ 1 ] * _rtB -> B_13_5_0 + _rtB -> B_20_1_0 [
1 ] ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB ->
B_20_16_0 [ 0 ] = _rtDW -> Memory_PreviousInput_b [ 0 ] ; _rtB -> B_20_17_0 [
0 ] = _rtDW -> Memory_PreviousInput_a [ 0 ] ; _rtB -> B_20_16_0 [ 1 ] = _rtDW
-> Memory_PreviousInput_b [ 1 ] ; _rtB -> B_20_17_0 [ 1 ] = _rtDW ->
Memory_PreviousInput_a [ 1 ] ; ssCallAccelRunBlock ( S , 16 , 0 ,
SS_CALL_MDL_OUTPUTS ) ; if ( _rtB -> B_16_0_1 ) { ssSetStopRequested ( S , 1
) ; } _rtB -> B_20_21_0 = _rtDW -> UnitDelay_DSTATE ; if ( _rtB -> B_20_21_0
> 0.0 ) { _rtB -> B_17_1_0 = _rtB -> B_20_9_0 + _rtDW -> UnitDelay_DSTATE_n ;
if ( ssIsModeUpdateTimeStep ( S ) ) { srUpdateBC ( _rtDW ->
cumsum_SubsysRanBC ) ; } } } _rtB -> B_20_24_0 = ( _rtB -> B_15_1_0 - _rtB ->
B_13_5_0 ) / _rtB -> B_13_5_0 ; if ( ssIsModeUpdateTimeStep ( S ) ) { _rtDW
-> Abs_MODE = ( _rtB -> B_20_24_0 >= 0.0 ) ; } if ( ssIsModeUpdateTimeStep (
S ) ) { _rtDW -> Switch1_Mode = ( _rtB -> B_20_6_0 >= _rtP -> P_64 ) ; } if (
_rtDW -> Switch1_Mode ) { _rtB -> B_18_0_0 = _rtB -> B_20_5_0_c - ( _rtDW ->
Abs_MODE > 0 ? _rtB -> B_20_24_0 : - _rtB -> B_20_24_0 ) ; _rtB -> B_20_27_0
= _rtB -> B_18_0_0 ; } else { _rtB -> B_20_27_0 = _rtB -> B_20_6_0_k ; } if (
ssIsModeUpdateTimeStep ( S ) ) { _rtB -> B_20_28_0 = _rtB -> B_20_27_0 ;
_rtDW -> MinMax_MODE = 0 ; if ( ( _rtB -> B_20_27_0 != _rtB -> B_20_27_0 ) ||
( _rtB -> B_20_4_0 < _rtB -> B_20_27_0 ) ) { _rtB -> B_20_28_0 = _rtB ->
B_20_4_0 ; _rtDW -> MinMax_MODE = 1 ; } } else { _rtB -> B_20_28_0 = _rtB ->
B_20_27_0 ; if ( _rtDW -> MinMax_MODE == 1 ) { _rtB -> B_20_28_0 = _rtB ->
B_20_4_0 ; } } UNUSED_PARAMETER ( tid ) ; } static void mdlOutputsTID4 (
SimStruct * S , int_T tid ) { B_AbstractFuelControl_M1_LOKI_T * _rtB ;
P_AbstractFuelControl_M1_LOKI_T * _rtP ; _rtP = ( (
P_AbstractFuelControl_M1_LOKI_T * ) ssGetModelRtp ( S ) ) ; _rtB = ( (
B_AbstractFuelControl_M1_LOKI_T * ) _ssGetModelBlockIO ( S ) ) ; _rtB ->
B_20_0_0 = _rtP -> P_65 ; _rtB -> B_20_1_0 [ 0 ] = _rtP -> P_66 [ 0 ] ; _rtB
-> B_20_1_0 [ 1 ] = _rtP -> P_66 [ 1 ] ; _rtB -> B_8_0_0 = _rtP -> P_95 ;
_rtB -> B_8_2_0 = _rtB -> B_8_0_0 + _rtP -> P_96 ; _rtB -> B_9_0_0 = _rtP ->
P_97 ; _rtB -> B_4_0_0 = _rtP -> P_88 ; _rtB -> B_4_1_0 = _rtP -> P_89 ; _rtB
-> B_15_1_0_b = _rtP -> P_47 ; _rtB -> B_15_2_0 = _rtP -> P_48 ; _rtB ->
B_15_3_0_p = _rtP -> P_49 ; _rtB -> B_15_4_0_c = _rtP -> P_50 ; _rtB ->
B_15_5_0 = _rtP -> P_51 ; _rtB -> B_20_3_0 = _rtP -> P_67 ; _rtB ->
B_20_4_0_m = _rtP -> P_68 ; _rtB -> B_20_5_0_c = _rtP -> P_69 ; _rtB ->
B_20_6_0_k = _rtP -> P_70 ; _rtB -> B_20_7_0_c = _rtP -> P_71 ; _rtB ->
B_20_8_0 [ 0 ] = _rtP -> P_72 [ 0 ] ; _rtB -> B_20_8_0 [ 1 ] = _rtP -> P_72 [
1 ] ; UNUSED_PARAMETER ( tid ) ; }
#define MDL_UPDATE
static void mdlUpdate ( SimStruct * S , int_T tid ) {
B_AbstractFuelControl_M1_LOKI_T * _rtB ; DW_AbstractFuelControl_M1_LOKI_T *
_rtDW ; P_AbstractFuelControl_M1_LOKI_T * _rtP ;
X_AbstractFuelControl_M1_LOKI_T * _rtX ; int32_T isHit ; _rtDW = ( (
DW_AbstractFuelControl_M1_LOKI_T * ) ssGetRootDWork ( S ) ) ; _rtX = ( (
X_AbstractFuelControl_M1_LOKI_T * ) ssGetContStates ( S ) ) ; _rtP = ( (
P_AbstractFuelControl_M1_LOKI_T * ) ssGetModelRtp ( S ) ) ; _rtB = ( (
B_AbstractFuelControl_M1_LOKI_T * ) _ssGetModelBlockIO ( S ) ) ; isHit =
ssIsSampleHit ( S , 1 , 0 ) ; if ( isHit != 0 ) { _rtDW ->
Memory_PreviousInput [ 0 ] = _rtB -> B_16_0_0 [ 0 ] ; _rtDW ->
Memory_PreviousInput [ 1 ] = _rtB -> B_16_0_0 [ 1 ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> fuelsystemtransportdelay_PWORK . TUbufferPtrs [ 0
] ; real_T simTime = ssGetT ( S ) ; _rtDW -> fuelsystemtransportdelay_IWORK .
Head = ( ( _rtDW -> fuelsystemtransportdelay_IWORK . Head < ( _rtDW ->
fuelsystemtransportdelay_IWORK . CircularBufSize - 1 ) ) ? ( _rtDW ->
fuelsystemtransportdelay_IWORK . Head + 1 ) : 0 ) ; if ( _rtDW ->
fuelsystemtransportdelay_IWORK . Head == _rtDW ->
fuelsystemtransportdelay_IWORK . Tail ) { if ( !
AbstractFuelControl_M1_LOKI_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
fuelsystemtransportdelay_IWORK . CircularBufSize , & _rtDW ->
fuelsystemtransportdelay_IWORK . Tail , & _rtDW ->
fuelsystemtransportdelay_IWORK . Head , & _rtDW ->
fuelsystemtransportdelay_IWORK . Last , simTime - _rtP -> P_38 , uBuffer , (
boolean_T ) 0 , ( boolean_T ) 1 , & _rtDW -> fuelsystemtransportdelay_IWORK .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "vtdelay memory allocation error"
) ; return ; } } ( * uBuffer + _rtDW -> fuelsystemtransportdelay_IWORK .
CircularBufSize ) [ _rtDW -> fuelsystemtransportdelay_IWORK . Head ] =
simTime ; ( * uBuffer + 2 * _rtDW -> fuelsystemtransportdelay_IWORK .
CircularBufSize ) [ _rtDW -> fuelsystemtransportdelay_IWORK . Head ] = ( (
X_AbstractFuelControl_M1_LOKI_T * ) ssGetContStates ( S ) ) ->
fuelsystemtransportdelay_CSTATE ; ( * uBuffer ) [ _rtDW ->
fuelsystemtransportdelay_IWORK . Head ] = _rtB -> B_15_45_0 ; } isHit =
ssIsSampleHit ( S , 1 , 0 ) ; if ( isHit != 0 ) { _rtDW ->
Memory_PreviousInput_e = _rtB -> B_20_28_0 ; } isHit = ssIsSampleHit ( S , 2
, 0 ) ; if ( isHit != 0 ) { _rtDW -> Memory_PreviousInput_b [ 0 ] = _rtB ->
B_20_15_0 [ 0 ] ; _rtDW -> Memory_PreviousInput_b [ 1 ] = _rtB -> B_20_15_0 [
1 ] ; _rtDW -> Memory_PreviousInput_a [ 0 ] = _rtB -> B_16_0_0 [ 0 ] ; _rtDW
-> Memory_PreviousInput_a [ 1 ] = _rtB -> B_16_0_0 [ 1 ] ; _rtDW ->
UnitDelay_DSTATE = _rtB -> B_20_3_0 ; if ( _rtB -> B_20_21_0 > 0.0 ) { _rtDW
-> UnitDelay_DSTATE_n = _rtB -> B_17_1_0 ; } } UNUSED_PARAMETER ( tid ) ; }
#define MDL_UPDATE
static void mdlUpdateTID4 ( SimStruct * S , int_T tid ) { UNUSED_PARAMETER (
tid ) ; }
#define MDL_DERIVATIVES
static void mdlDerivatives ( SimStruct * S ) {
B_AbstractFuelControl_M1_LOKI_T * _rtB ; DW_AbstractFuelControl_M1_LOKI_T *
_rtDW ; P_AbstractFuelControl_M1_LOKI_T * _rtP ;
XDot_AbstractFuelControl_M1_LOKI_T * _rtXdot ;
X_AbstractFuelControl_M1_LOKI_T * _rtX ; _rtDW = ( (
DW_AbstractFuelControl_M1_LOKI_T * ) ssGetRootDWork ( S ) ) ; _rtXdot = ( (
XDot_AbstractFuelControl_M1_LOKI_T * ) ssGetdX ( S ) ) ; _rtX = ( (
X_AbstractFuelControl_M1_LOKI_T * ) ssGetContStates ( S ) ) ; _rtP = ( (
P_AbstractFuelControl_M1_LOKI_T * ) ssGetModelRtp ( S ) ) ; _rtB = ( (
B_AbstractFuelControl_M1_LOKI_T * ) _ssGetModelBlockIO ( S ) ) ; _rtXdot ->
Integrator_CSTATE = _rtB -> B_15_29_0 ; _rtXdot -> Throttledelay_CSTATE = 0.0
; _rtXdot -> Throttledelay_CSTATE += _rtP -> P_9 * _rtX ->
Throttledelay_CSTATE ; _rtXdot -> Throttledelay_CSTATE += _rtB -> B_20_2_0 [
0 ] ; _rtXdot -> p00543bar_CSTATE = _rtB -> B_15_52_0 ; _rtXdot ->
Integrator_CSTATE_h = _rtB -> B_15_48_0 ; _rtXdot -> Integrator_CSTATE_c =
_rtB -> B_15_56_0 ; { real_T instantDelay ; instantDelay = _rtB -> B_15_50_0
; if ( instantDelay > _rtP -> P_38 ) { instantDelay = _rtP -> P_38 ; } if (
instantDelay < 0.0 ) { ( ( XDot_AbstractFuelControl_M1_LOKI_T * ) ssGetdX ( S
) ) -> fuelsystemtransportdelay_CSTATE = 0 ; } else { ( (
XDot_AbstractFuelControl_M1_LOKI_T * ) ssGetdX ( S ) ) ->
fuelsystemtransportdelay_CSTATE = 1.0 / instantDelay ; } } }
#define MDL_ZERO_CROSSINGS
static void mdlZeroCrossings ( SimStruct * S ) {
B_AbstractFuelControl_M1_LOKI_T * _rtB ; DW_AbstractFuelControl_M1_LOKI_T *
_rtDW ; P_AbstractFuelControl_M1_LOKI_T * _rtP ;
ZCV_AbstractFuelControl_M1_LOKI_T * _rtZCSV ; _rtDW = ( (
DW_AbstractFuelControl_M1_LOKI_T * ) ssGetRootDWork ( S ) ) ; _rtZCSV = ( (
ZCV_AbstractFuelControl_M1_LOKI_T * ) ssGetSolverZcSignalVector ( S ) ) ;
_rtP = ( ( P_AbstractFuelControl_M1_LOKI_T * ) ssGetModelRtp ( S ) ) ; _rtB =
( ( B_AbstractFuelControl_M1_LOKI_T * ) _ssGetModelBlockIO ( S ) ) ; _rtZCSV
-> theta090_UprLim_ZC = _rtB -> B_15_3_0 - _rtP -> P_11 ; _rtZCSV ->
theta090_LwrLim_ZC = _rtB -> B_15_3_0 - _rtP -> P_12 ; _rtZCSV ->
AFSensorFaultInjection_StepTime_ZC = ssGetT ( S ) - _rtP -> P_16 ; if ( (
_rtB -> B_15_16_0 != _rtB -> B_15_16_0 ) || ( _rtB -> B_15_15_0 < _rtB ->
B_15_16_0 ) ) { if ( _rtDW -> MinMax_MODE_c == 0 ) { _rtZCSV ->
MinMax_MinmaxInput_ZC_j = _rtB -> B_15_15_0 - _rtB -> B_15_15_0 ; } else {
_rtZCSV -> MinMax_MinmaxInput_ZC_j = _rtB -> B_15_15_0 - _rtB -> B_15_16_0 ;
} } else if ( _rtDW -> MinMax_MODE_c == 0 ) { _rtZCSV ->
MinMax_MinmaxInput_ZC_j = _rtB -> B_15_16_0 - _rtB -> B_15_15_0 ; } else {
_rtZCSV -> MinMax_MinmaxInput_ZC_j = _rtB -> B_15_16_0 - _rtB -> B_15_16_0 ;
} _rtZCSV -> Switch_SwitchCond_ZC = _rtB -> B_15_17_0 - _rtP -> P_22 ;
_rtZCSV -> flowdirection_Input_ZC = _rtB -> B_15_20_0 ; _rtZCSV ->
Pwon_StepTime_ZC = ssGetT ( S ) - _rtP -> P_0 ; _rtZCSV ->
Switch2_SwitchCond_ZC = _rtB -> B_20_6_0 - _rtP -> P_58 ; _rtZCSV ->
Switch3_SwitchCond_ZC = _rtB -> B_20_7_0 - _rtP -> P_59 ; _rtZCSV ->
Compare_RelopInput_ZC = _rtB -> B_20_6_0 - _rtB -> B_20_0_0 ; _rtZCSV ->
Abs_AbsZc_ZC = _rtB -> B_20_24_0 ; _rtZCSV -> Switch1_SwitchCond_ZC = _rtB ->
B_20_6_0 - _rtP -> P_64 ; if ( ( _rtB -> B_20_4_0 != _rtB -> B_20_4_0 ) || (
_rtB -> B_20_27_0 < _rtB -> B_20_4_0 ) ) { if ( _rtDW -> MinMax_MODE == 0 ) {
_rtZCSV -> MinMax_MinmaxInput_ZC = _rtB -> B_20_27_0 - _rtB -> B_20_27_0 ; }
else { _rtZCSV -> MinMax_MinmaxInput_ZC = _rtB -> B_20_27_0 - _rtB ->
B_20_4_0 ; } } else if ( _rtDW -> MinMax_MODE == 0 ) { _rtZCSV ->
MinMax_MinmaxInput_ZC = _rtB -> B_20_4_0 - _rtB -> B_20_27_0 ; } else {
_rtZCSV -> MinMax_MinmaxInput_ZC = _rtB -> B_20_4_0 - _rtB -> B_20_4_0 ; } }
static void mdlInitializeSizes ( SimStruct * S ) { ssSetChecksumVal ( S , 0 ,
726328350U ) ; ssSetChecksumVal ( S , 1 , 2217032959U ) ; ssSetChecksumVal (
S , 2 , 1682936618U ) ; ssSetChecksumVal ( S , 3 , 3496924551U ) ; { mxArray
* slVerStructMat = ( NULL ) ; mxArray * slStrMat = mxCreateString (
"simulink" ) ; char slVerChar [ 10 ] ; int status = mexCallMATLAB ( 1 , &
slVerStructMat , 1 , & slStrMat , "ver" ) ; if ( status == 0 ) { mxArray *
slVerMat = mxGetField ( slVerStructMat , 0 , "Version" ) ; if ( slVerMat == (
NULL ) ) { status = 1 ; } else { status = mxGetString ( slVerMat , slVerChar
, 10 ) ; } } mxDestroyArray ( slStrMat ) ; mxDestroyArray ( slVerStructMat )
; if ( ( status == 1 ) || ( strcmp ( slVerChar , "10.7" ) != 0 ) ) { return ;
} } ssSetOptions ( S , SS_OPTION_EXCEPTION_FREE_CODE ) ; if (
ssGetSizeofDWork ( S ) != ( SLSize ) sizeof (
DW_AbstractFuelControl_M1_LOKI_T ) ) { static char msg [ 256 ] ; sprintf (
msg , "Unexpected error: Internal DWork sizes do "
"not match for accelerator mex file (%ld vs %lu)." , ( signed long )
ssGetSizeofDWork ( S ) , ( unsigned long ) sizeof (
DW_AbstractFuelControl_M1_LOKI_T ) ) ; ssSetErrorStatus ( S , msg ) ; } if (
ssGetSizeofGlobalBlockIO ( S ) != ( SLSize ) sizeof (
B_AbstractFuelControl_M1_LOKI_T ) ) { static char msg [ 256 ] ; sprintf ( msg
, "Unexpected error: Internal BlockIO sizes do "
"not match for accelerator mex file (%ld vs %lu)." , ( signed long )
ssGetSizeofGlobalBlockIO ( S ) , ( unsigned long ) sizeof (
B_AbstractFuelControl_M1_LOKI_T ) ) ; ssSetErrorStatus ( S , msg ) ; } { int
ssSizeofParams ; ssGetSizeofParams ( S , & ssSizeofParams ) ; if (
ssSizeofParams != sizeof ( P_AbstractFuelControl_M1_LOKI_T ) ) { static char
msg [ 256 ] ; sprintf ( msg ,
"Unexpected error: Internal Parameters sizes do "
"not match for accelerator mex file (%d vs %lu)." , ssSizeofParams , (
unsigned long ) sizeof ( P_AbstractFuelControl_M1_LOKI_T ) ) ;
ssSetErrorStatus ( S , msg ) ; } } _ssSetModelRtp ( S , ( real_T * ) &
AbstractFuelControl_M1_LOKI_rtDefaultP ) ; rt_InitInfAndNaN ( sizeof ( real_T
) ) ; ( ( P_AbstractFuelControl_M1_LOKI_T * ) ssGetModelRtp ( S ) ) -> P_73 =
rtInfF ; } static void mdlInitializeSampleTimes ( SimStruct * S ) {
slAccRegPrmChangeFcn ( S , mdlOutputsTID4 ) ; } static void mdlTerminate (
SimStruct * S ) { }
#include "simulink.c"
