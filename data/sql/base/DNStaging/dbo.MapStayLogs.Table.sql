USE [dnstaging]
GO
/****** Object:  Table [dbo].[MapStayLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[MapStayLogs](
	[ExitDate] [datetime2](0) NOT NULL,
	[MapID] [int] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[EntranceDate] [datetime2](0) NOT NULL
) ON [PRIMARY]
GO
