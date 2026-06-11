# 🎓 EduSystem — Education Management Platform

![Next.js](https://img.shields.io/badge/Next.js-14-black?style=for-the-badge&logo=next.js)
![React](https://img.shields.io/badge/React-18-61DAFB?style=for-the-badge&logo=react)
![Tailwind CSS](https://img.shields.io/badge/Tailwind_CSS-3-38B2AC?style=for-the-badge&logo=tailwind-css)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)

A full-featured **Education Management System** built with Next.js, offering three dedicated dashboards for Admins, Instructors, and Students — all in one unified platform.

---

## 📸 Preview

> _Screenshots or demo GIF can be placed here_

---

## 📋 Table of Contents

- [Features](#-features)
- [Dashboards](#-dashboards)
- [Pages](#-pages)
- [Getting Started](#-getting-started)
- [Project Structure](#-project-structure)

---

## ✨ Features

- 🔐 Role-based authentication (Admin / Instructor / Student)
- 📊 Three dedicated dashboards with tailored views
- 🗓️ Interactive timetable management
- 🏫 Room Details and availability Rooms
- 👥 Admin management for admins, instructors, and students
- 📱 Fully responsive design

---

## 🖥️ Dashboards

The system provides three separate dashboards, each tailored to the user's role:

### 🛡️ Admin Dashboard
Full control over the platform. Admins can manage all users, rooms, schedules, and system settings.

| Capability | Description |
|---|---|
| Manage Admins | Add, edit, or remove admin accounts |
| Manage Instructors | Oversee instructor profiles and assignments |
| Manage Students | View and control student enrollments |
| Room Management | Create and configure classrooms |
| Feature Management | Create and configure Features |
| Timetable Control | Build and publish schedules for all courses |

---

### 👨‍🏫 Instructor Dashboard
Instructors can view their assigned classes, manage their schedules, and track student activity.

| Capability | Description |
|---|---|
| View Schedule | See personal timetable and assigned rooms |
| Students | View enrolled students per course |
| Rooms  | Check room availability |

---

### 🎓 Student Dashboard
Students can access their timetable, view course details, and stay up to date with their schedule.

| Capability | Description |
|---|---|
| View Timetable | See personal class schedule |
| Course Info | Browse available courses and instructors |
| Room Info | Know which room their classes are held in |

---

## 📄 Pages

| Page | Route | Description |
|---|---|---|
| 🏠 Home | `/` | Landing page with platform overview |
| 🔑 Login | `/login` | Unified login with role-based redirect |
| 🚪 Logout | `/logout` | Secure session logout |
| 🏫 Rooms | `/rooms` | View and manage available rooms |
| ⭐ Features | `/features` | Highlights of platform capabilities |
| 🗓️ Timetable | `/timetable` | Schedule viewer and editor |
| 🛡️ Admins | `/admins` | Admin management (Admin only) |
| 👨‍🏫 Instructors | `/instructors` | Instructor directory and management |
| 🎓 Students | `/students` | Student roster and management |

---

## 🛠️ Tech Stack

- **Framework:** [Next.js 14](https://nextjs.org/) (App Router)
- **UI Library:** [React 18](https://react.dev/)
- **Styling:** [Tailwind CSS](https://tailwindcss.com/)
- **Auth:** [NextAuth.js](https://next-auth.js.org/) with role-based access
- **Icons:** [Lucide React](https://lucide.dev/)
- **Forms:** [React Hook Form](https://react-hook-form.com/)

---

## 🚀 Getting Started

### Prerequisites

- Node.js `v18+`
- npm or yarn
- PostgreSQL (or any supported database)

### Installation

```bash
# 1. Clone the repository
git https://github.com/Asemhamed/GraduationProject-FE
cd GraduationProject-FE

# 2. Install dependencies
npm install

# 3. Set up environment variables
cp .env.example .env.local
```



# App
NEXT_PUBLIC_APP_NAME="EduSystem"
```


Open [http://localhost:3000](http://localhost:3000) in your browser.

---

## 📁 Project Structure

```
edu-system/
├── app/
│   ├── (auth)/
│   │   ├── login/
│   │      └── page.jsx
│   │       
│   ├── (dashboard)/
│   │   ├── admin/
│   │   │   ├── _Components/
│   │   │   ├── admins/
│   │   │   ├── instructors/
│   │   │   ├── students/
│   │   │   ├── rooms/
│   │   │   ├── features/
│   │   │   ├── profile/
│   │   │   └── timetable/
│   │   ├── instructor/
│   │   │   ├── _Components/
│   │   │   ├── timetable/
│   │   │   └── profile/
│   │   └── student/
│   │   │   ├── _Components/
│   │   │   ├── profile/
│   │   │   ├── enrollment/
│   │   |   └── timetable/
│   ├── layout.tsx
│   └── page.tsx              ← Home page
├── components/
│   ├── ui/
│   ├── layout/
│   │   ├── Navbar.tsx
│   │   └── Sidebar.tsx
├── Hooks/
│   ├── useLogout
│   └── useMobile
├── Cookies/
│   └── auth.actions.tsx
├── Context/
│   └── userData.tsx
├── Types/
│   ├── AdminTypes.tsx
│   ├── StudentTypes.tsx
│   ├── InstructorTypes.tsx
│   ├── RoomTypes.tsx
│   ├── FeatureTypes.tsx
│   ├── CourseTypes.tsx
│   └── TimetableTypes.tsx
├── ServerAcions/
│   ├── Admin/
│   ├── Course/
│   ├── Enrollment/
│   ├── Feature/
│   ├── Instructor/
│   ├── Profile/
│   ├── Rooms/
│   ├── Student/
│   └── Timetable/
├── Schema/
│   ├── AuthSchema.tsx
│   └── StudentSchema.tsx
├── lib/
│   ├── auth.js
│   └── db.js
├── prisma/
│   └── schema.prisma
├── public/
├── .env.example
├── next.config.js
├── tailwind.config.js
└── README.md
```

---

## 🔐 Default Credentials (Dev/Seed)

> ⚠️ **Change these in production!**

| Role | Email | Password |
|---|---|---|
| Admin | admin@admin.com | `admin` |


---

## 🤝 Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository
2. Create your feature branch: `git checkout -b feature/your-feature`
3. Commit your changes: `git commit -m 'Add: your feature description'`
4. Push to the branch: `git push origin feature/your-feature`
5. Open a Pull Request

Please make sure your code follows the existing style and passes linting:

```bash
npm run lint
```

---

## 📜 License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

---

## 🙌 Acknowledgements

- [Next.js Documentation](https://nextjs.org/docs)
- [Tailwind CSS](https://tailwindcss.com/)
- [NextAuth.js](https://next-auth.js.org/)
- [Prisma](https://www.prisma.io/)

---

<p align="center">Made with ❤️ for better education</p>