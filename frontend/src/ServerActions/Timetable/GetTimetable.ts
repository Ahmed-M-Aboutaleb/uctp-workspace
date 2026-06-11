'use server'

import { getToken } from "@/cookies/auth.actions";
import { TimetableResponse } from "@/Types/TimetableTypes";


export async function GetTimetable(): Promise<TimetableResponse> {
  const token = await getToken();
  try {
    const response = await fetch(`${process.env.API_URL}/api/timetable/`, {
      method: "GET",
      headers: {
        "Accept": "application/json",
        "Authorization": `Bearer ${token}`,
      },
    })


    const data = await response.json()
    
    return data
  } catch (error) {
    console.error(" Error fetching timetable from backend:", error)
    throw error
    
  }
}
