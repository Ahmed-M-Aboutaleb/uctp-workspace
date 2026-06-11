export type CreateAdminData = {
  full_name: string,
  title: string,
  department: string,
  email: string,
  password: string
}

export type CreateAdminResponse = {
  admin_id: number,
  full_name: string,
  title: string,
  department: string,
  user_id: number
}

export type UpdateAdminData = {
  full_name: string,
  title: string,
  department: string,
}
export type UpdateProfileData = {
  full_name: string,
  title: string,
  department: string,
  semester: string,
}


export type AdminResponse = {
  admin_id: number,
  full_name: string,
  title: string,
  department: string,
  user_id: number
}

