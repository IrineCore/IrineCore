USE [dnmembership]
GO
/****** Object:  Table [dbo].[WarpCharacters]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[WarpCharacters]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[WarpCharacters](
	[CharacterID] [bigint] NOT NULL,
 CONSTRAINT [PK_WarpCharacters] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
