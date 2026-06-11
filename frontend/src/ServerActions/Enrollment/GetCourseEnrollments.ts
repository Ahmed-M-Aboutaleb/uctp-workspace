'use server'

import { getToken } from "@/cookies/auth.actions";
import { StudentResponse } from "@/Types/StudentTypes";


export async function GetCourseEnrollments(course_Id:number): Promise<StudentResponse> {
  const token = await getToken();
  try {
    const response = await fetch(`${process.env.API_URL}/api/enrollment/${course_Id}/students`, {
      method: "GET",
      headers: {
        "Accept": "application/json",
        "Authorization": `Bearer ${token}`,
      },
    })


    const data = await response.json();
    console.log( );
    
    return data
  } catch (error) {
    console.error(" Error fetching course enrollments from backend:", error)
    throw error
    
  }
}
