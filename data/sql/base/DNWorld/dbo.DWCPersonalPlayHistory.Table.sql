USE [dnworld]
GO
/****** Object:  Table [dbo].[DWCPersonalPlayHistory]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DWCPersonalPlayHistory](
	[CharacterID] [bigint] NOT NULL,
	[DWCPersonalPlayHistoryID] [bigint] IDENTITY(1,1) NOT NULL,
	[DWCTeamID] [int] NOT NULL,
	[ResultCode] [tinyint] NOT NULL,
	[GainDWCPoint] [int] NOT NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_DWCPersonalPlayHistory] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[DWCPersonalPlayHistoryID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
