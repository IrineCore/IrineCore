USE [dnworld]
GO
/****** Object:  Table [dbo].[PastDWCTeams]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PastDWCTeams](
	[DWCSequence] [int] NULL,
	[DWCTeamID] [int] NOT NULL,
	[DWCTeamName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[Win] [int] NOT NULL,
	[Lose] [int] NOT NULL,
	[Draw] [int] NOT NULL,
	[DWCPoint] [int] NOT NULL,
	[CreateDate] [datetime2](0) NOT NULL
) ON [DataFG1]
GO
