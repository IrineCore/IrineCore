USE [dnworld]
GO
/****** Object:  Table [dbo].[CanceledGuildWarFinalResults]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CanceledGuildWarFinalResults](
	[GuildID] [int] NOT NULL,
	[GuildWarScheduleID] [smallint] NOT NULL,
	[MatchSequence] [tinyint] NOT NULL,
	[MatchTypeCode] [tinyint] NOT NULL,
	[MatchResultCode] [tinyint] NOT NULL,
	[RegisterDate] [datetime] NOT NULL,
	[ChangeDate] [datetime] NOT NULL,
 CONSTRAINT [PK_CanceledGuildWarFinalResults] PRIMARY KEY CLUSTERED 
(
	[GuildID] ASC,
	[GuildWarScheduleID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
