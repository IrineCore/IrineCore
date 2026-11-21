USE [dnstaging]
GO
/****** Object:  Table [dbo].[AppellationsBuffer]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AppellationsBuffer](
	[CharacterID] [bigint] NOT NULL,
	[AppellationID] [int] NOT NULL,
	[ChoiceFlag] [bit] NULL
) ON [PRIMARY]
GO
