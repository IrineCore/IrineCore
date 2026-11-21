USE [dnstaging]
GO
/****** Object:  Table [dbo].[TempSecondAuthStatus]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TempSecondAuthStatus](
	[AccountID] [int] NOT NULL,
	[SecondAuthCode] [tinyint] NOT NULL
) ON [PRIMARY]
GO
