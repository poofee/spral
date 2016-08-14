! No-GPU version
! Specifies exact implementation of user facing types
module spral_ssids_type_select
   use spral_ssids_akeep, only : &
      ssids_akeep => ssids_akeep_base
   use spral_ssids_fkeep, only : &
      ssids_fkeep => ssids_fkeep_base
   use spral_ssids_inform, only : &
      ssids_inform => ssids_inform_base
   implicit none
contains
   logical function detect_gpu()
      ! Not compiled with GPU support
      detect_gpu = .false.
   end function detect_gpu
end module spral_ssids_type_select
