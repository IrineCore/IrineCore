USE [dnworld]
GO
/****** Object:  Table [dbo].[DWCTeamPointPlayHistory]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DWCTeamPointPlayHistory](
	[DWCTeamID] [int] NOT NULL,
	[DWCTeamPointPlayHistoryID] [bigint] IDENTITY(1,1) NOT NULL,
	[vsDWCTeamID] [int] NULL,
	[DWCPointChangeCode] [tinyint] NOT NULL,
	[DWCPointBefore] [int] NOT NULL,
	[DWCPoint] [int] NOT NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_DWCTeamPointPlayHistory] PRIMARY KEY CLUSTERED 
(
	[DWCTeamID] ASC,
	[DWCTeamPointPlayHistoryID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
