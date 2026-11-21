USE [dngsm]
GO
/****** Object:  Table [dbo].[GMError]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GMError](
	[IDX] [int] IDENTITY(1,1) NOT NULL,
	[version] [nvarchar](100) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[time] [nvarchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[username] [nvarchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[process] [nvarchar](200) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[os] [nvarchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[cpu] [nvarchar](100) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[memory] [text] COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[video] [nvarchar](200) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[directx] [nvarchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[ip] [nvarchar](30) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[debugdata] [text] COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[userlog] [text] COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[address] [text] COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[register] [text] COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[stack] [text] COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[clientip] [nvarchar](100) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[filename] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[RegTime] [datetime] NULL
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
GO
