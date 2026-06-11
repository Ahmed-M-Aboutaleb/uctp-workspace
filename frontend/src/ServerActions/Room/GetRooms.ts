'use server'

import { getToken } from "@/cookies/auth.actions";
import { RoomsResponse } from "@/Types/RoomsType";

export async function GetRooms(skip: number = 0, limit: number = 100): Promise<RoomsResponse > {
    const token = await getToken();
  try {
    const response = await fetch(`${process.env.API_URL}/api/facilities/rooms?skip=${skip}&limit=${limit}`, {
      method: "GET",
      headers: {
        "Accept": "application/json",
        "Authorization": `Bearer ${token}`
      },
    });


    const data = await response.json()
    return data
  } catch (error) {
    console.error(" Error fetching rooms from backend:", error)
    throw error
  }
}