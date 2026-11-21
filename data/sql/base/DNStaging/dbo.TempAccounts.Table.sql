USE [dnstaging]
GO
/****** Object:  Table [dbo].[TempAccounts]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TempAccounts](
	[AccountID] [int] NOT NULL,
	[OldAccountName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[NewAccountName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL
) ON [PRIMARY]
GO
