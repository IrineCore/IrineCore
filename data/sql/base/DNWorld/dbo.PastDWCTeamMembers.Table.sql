USE [dnworld]
GO
/****** Object:  Table [dbo].[PastDWCTeamMembers]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PastDWCTeamMembers](
	[DWCSequence] [int] NULL,
	[DWCTeamID] [int] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[TeamLeaderFlag] [bit] NOT NULL,
	[RegisterDate] [datetime2](0) NOT NULL
) ON [DataFG1]
GO
