USE [dnworld]
GO
/****** Object:  Table [dbo].[PastDWCTeamPointPlayHistory]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PastDWCTeamPointPlayHistory](
	[DWCSequence] [int] NULL,
	[DWCTeamID] [int] NOT NULL,
	[vsDWCTeamID] [int] NULL,
	[DWCPointChangeCode] [tinyint] NOT NULL,
	[DWCPointBefore] [int] NOT NULL,
	[DWCPoint] [int] NOT NULL,
	[RegisterDate] [datetime2](0) NOT NULL
) ON [DataFG1]
GO
