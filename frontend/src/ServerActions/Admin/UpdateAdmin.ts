'use server'

import { getToken } from "@/cookies/auth.actions";
import { CreateAdminResponse, UpdateAdminData } from "@/Types/AdminTypes";

export async function UpdateAdmin(admin_id: number, admin_data: UpdateAdminData): Promise<CreateAdminResponse> {
    const token = await getToken();
    try {
    const response = await fetch(`${process.env.API_URL}/api/people/admins/${admin_id}`, {
        method: "PATCH",
        headers: {
            'Content-Type': 'application/json',
            'Accept': 'application/json',
            'Authorization': `Bearer ${token}`
        },
        body: JSON.stringify(admin_data)
    });

    if (!response.ok) {
        throw new Error("Failed to update admin")
    }
    const data = await response.json();
    return data;
    
  } catch (error) {
    console.error("Error updating admin:", error)
    throw error
}
}