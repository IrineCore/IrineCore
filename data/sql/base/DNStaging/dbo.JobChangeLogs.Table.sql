USE [dnstaging]
GO
/****** Object:  Table [dbo].[JobChangeLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[JobChangeLogs](
	[LogDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[JobCode] [tinyint] NOT NULL
) ON [PRIMARY]
GO
