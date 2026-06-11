export type CreateInstructorData = {
  name: string,
  email: string,
  password: string
}

export type CreateInstructorResponse = {
  instructor_id: number,
  name: string,
  user_id: number
}

export type InstructorResponse = {
  instructor_id: number,
  name: string,
  user_id: number
}

export type GetInstructorsResponse =  InstructorResponse[]
