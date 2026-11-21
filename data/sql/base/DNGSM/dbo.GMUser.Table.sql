USE [dngsm]
GO
/****** Object:  Table [dbo].[GMUser]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[GMUser](
	[UserIndex] [int] NOT NULL,
	[UserID] [nvarchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[UserName] [nvarchar](10) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[UserPassword] [nvarchar](100) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[GroupID] [int] NOT NULL,
	[Email] [varchar](100) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[Memo] [nvarchar](200) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[LastLoginTime] [datetime] NULL,
	[IsDelete] [bit] NULL,
	[ConnectIP] [varchar](15) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY]
GO
SET ANSI_PADDING OFF
GO
