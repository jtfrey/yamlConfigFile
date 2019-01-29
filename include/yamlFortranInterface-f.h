!
! YAML Fortran Interface
!

!
! SUBROUTINE YAMLConfigFile_Open(
!       character(len=*), intent(in)    :: filename,
!       integer, intent(inout)          :: yamlUnit
! )
!
! SUBROUTINE YAMLConfigFile_Close(
!       integer, intent(in)             :: yamlUnit
! )
!
! SUBROUTINE YAMLConfigFile_CloseAll()
!
! SUBROUTINE YAMLConfigFile_Summary()
!

!
! FUNCTION YAMLConfigFile_GetNodeIsType(
!       integer, intent(in)             :: yamlUnit,
!       character(len=*), intent(in)    :: keyPath,
!       character(len=*), intent(in)    :: type,
!       integer, intent(out)            :: ierr
! ) RESULT(logical)
!
! Where type can be:
!    scalar | sequence | list | array | mapping | dict | dictionary | hash
!
LOGICAL         :: YAMLConfigFile_GetNodeIsType

!
! FUNCTION YAMLConfigFile_GetNodeCollectionSize(
!       integer, intent(in)             :: yamlUnit,
!       character(len=*), intent(in)    :: keyPath,
!       integer, intent(out)            :: ierr
! ) RESULT(integer)
!
INTEGER         :: YAMLConfigFile_GetNodeCollectionSize

!
! FUNCTION YAMLConfigFile_Get*(
!       integer, intent(in)             :: yamlUnit,
!       character(len=*), intent(in)    :: keyPath,
!       <type>, intent(out)             :: value,
!       integer, intent(out)            :: ierr
! ) RESULT(logical)
!
LOGICAL         :: YAMLConfigFile_GetString
LOGICAL         :: YAMLConfigFile_GetInteger
LOGICAL         :: YAMLConfigFile_GetInteger4
LOGICAL         :: YAMLConfigFile_GetInteger8
LOGICAL         :: YAMLConfigFile_GetReal
LOGICAL         :: YAMLConfigFile_GetReal4
LOGICAL         :: YAMLConfigFile_GetReal8
LOGICAL         :: YAMLConfigFile_GetDoublePrecision
LOGICAL         :: YAMLConfigFile_GetLogical

!
! FUNCTION YAMLConfigFile_Get*Array(
!       integer, intent(in)             :: yamlUnit,
!       character(len=*), intent(in)    :: keyPath,
!       integer, intent(in)             :: startIndex,
!       integer, intent(in)             :: endIndex,
!       <type>, intent(out)             :: array(:),
!       integer, intent(out)            :: ierr
! ) RESULT(logical)
!
LOGICAL         :: YAMLConfigFile_GetIntegerArray
LOGICAL         :: YAMLConfigFile_GetInteger4Array
LOGICAL         :: YAMLConfigFile_GetInteger8Array
LOGICAL         :: YAMLConfigFile_GetRealArray
LOGICAL         :: YAMLConfigFile_GetReal4Array
LOGICAL         :: YAMLConfigFile_GetReal8Array
LOGICAL         :: YAMLConfigFile_GetDoublePrecisionArray
LOGICAL         :: YAMLConfigFile_GetLogicalArray

