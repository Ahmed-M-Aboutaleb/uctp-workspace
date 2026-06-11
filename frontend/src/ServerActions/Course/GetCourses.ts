'use server'

import { getToken } from "@/cookies/auth.actions";
import { CourseResponse } from "@/Types/CourseTypes";


export async function GetCourses(skip: number = 0, limit: number = 100): Promise<CourseResponse> {
  const token = await getToken();
  try {
    const response = await fetch(`${process.env.API_URL}/api/courses?skip=${skip}&limit=${limit}`, {
      method: "GET",
      headers: {
        "Accept": "application/json",
        "Authorization": `Bearer ${token}`,
      },
    })



    const data = await response.json()
    
    return data
  } catch (error) {
    console.error(" Error fetching courses from backend:", error)
    throw error
    
  }
}
