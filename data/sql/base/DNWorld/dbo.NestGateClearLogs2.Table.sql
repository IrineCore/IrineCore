USE [dnworld]
GO
/****** Object:  Table [dbo].[NestGateClearLogs2]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[NestGateClearLogs2](
	[NestGateStartID] [bigint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[ClearFlag] [bit] NOT NULL
) ON [DataFG1]
GO
