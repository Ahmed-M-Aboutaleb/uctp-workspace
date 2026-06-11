import z from "zod";

export const StudentSchema = z.object({
    full_name: z.string().nonempty('Full name is required'),
    email: z.email('Invalid email address'),
    password: z.string().nonempty('Password is required'),
    semester: z.string().nonempty('Semester is required')
});
    
export type StudentType = z.infer<typeof StudentSchema>;  


