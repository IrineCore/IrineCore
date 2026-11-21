USE [dnserverlog]
GO
/****** Object:  Table [dbo].[SpecificLogs01]    Script Date: 11/14/2026 8:57:59 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SpecificLogs01](
	[LogDate] [datetime2](0) NOT NULL,
	[AccountID] [int] NOT NULL,
	[LogText] [nvarchar](2000) COLLATE Chinese_PRC_CI_AS NOT NULL
) ON [PRIMARY]
GO
