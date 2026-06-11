'use server'

import { revalidatePath } from "next/cache"
import { getToken } from "@/cookies/auth.actions"
import { CreateAdminResponse, UpdateProfileData } from "@/Types/AdminTypes"

export async function UpdateProfile(profile_data: UpdateProfileData): Promise<CreateAdminResponse> {
    const token = await getToken()

    const response = await fetch(`${process.env.API_URL}/api/people/me`, {
        method: "PUT",
        headers: {
            'Content-Type': 'application/json',
            'Accept': 'application/json',
            'Authorization': `Bearer ${token}`
        },
        body: JSON.stringify(profile_data),
        cache: "no-store"
    })

    if (!response.ok) {
        const errorBody = await response.json().catch(() => null)
        throw new Error(errorBody?.message || "Failed to update Profile")
    }

    const data = await response.json()

    revalidatePath("/profile") 

    return data
}