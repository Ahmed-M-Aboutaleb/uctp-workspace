'use server'

import { getToken } from "@/cookies/auth.actions";
import { Room } from "@/Types/RoomsType";
import { revalidatePath } from "next/cache";


export async function CreateRoom({capacity, feature_ids}: { capacity: number, feature_ids: number[] }): Promise<Room > {
  const token = await getToken();
  
  try {
    const response = await fetch(`${process.env.API_URL}/api/facilities/rooms`, {
      method: "POST",
      headers: {
        "Accept": "application/json",
        "Authorization": `Bearer ${token}`,
        "Content-Type": 'application/json'
      },
    body: JSON.stringify({ capacity, feature_ids })
    })

    if (!response.ok) {
      throw new Error(`API responded with status ${response.status}`)
    }

    const data = await response.json()
        revalidatePath("/admin");
        revalidatePath("/instructor");
        revalidatePath("/student");
    
    return data
  } catch (error) {
    console.error(" Error creating room:", error)
    throw new Error("Failed to create room" + (error instanceof Error ? `: ${error.message}` : ""))
  }
}
