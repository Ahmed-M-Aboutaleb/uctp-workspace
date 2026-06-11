'use server'

import { getToken } from "@/cookies/auth.actions";
import { revalidatePath } from "next/cache";

export async function UpdateRoom(room_id: number, capacity: number, feature_ids: number[]): Promise<any> {
    const token = await getToken();
    try {
    const response = await fetch(`${process.env.API_URL}/api/facilities/rooms/${room_id}`, {
        method: "PATCH",
        headers: {
            'Content-Type': 'application/json',
            'Accept': 'application/json',
            'Authorization': `Bearer ${token}`
        },
        body: JSON.stringify({ capacity, feature_ids })
    });

    if (!response.ok) {
        throw new Error("Failed to update room")
    }
    const data = await response.json();
        revalidatePath("/admin");
        revalidatePath("/instructor");
        revalidatePath("/student");
    
    return data;
    
  } catch (error) {
    console.error("Error updating room:", error)
    throw error
}
}