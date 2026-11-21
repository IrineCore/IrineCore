USE [dnmembership]
GO
/****** Object:  Table [dbo].[DNEvents]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DNEvents]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[DNEvents](
	[DNEventID] [int] IDENTITY(1,1) NOT NULL,
	[DNEventDetailID] [int] NOT NULL,
	[WorldID] [tinyint] NOT NULL,
	[EnableFlag] [bit] NOT NULL,
	[RegistrationDate] [smalldatetime] NOT NULL,
	[DisplayFlag] [bit] NOT NULL,
 CONSTRAINT [PK_DNEvents] PRIMARY KEY CLUSTERED 
(
	[DNEventID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_DNEvents_EnableFlag]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[DNEvents] ADD  CONSTRAINT [DF_DNEvents_EnableFlag]  DEFAULT ((1)) FOR [EnableFlag]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_DNEvents_RegistrationDate]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[DNEvents] ADD  CONSTRAINT [DF_DNEvents_RegistrationDate]  DEFAULT (getdate()) FOR [RegistrationDate]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_DNEvents_DisplayFlag]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[DNEvents] ADD  CONSTRAINT [DF_DNEvents_DisplayFlag]  DEFAULT ((0)) FOR [DisplayFlag]
END
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_DNEventDetails_DNEvents_DNEventDetailID]') AND parent_object_id = OBJECT_ID(N'[dbo].[DNEvents]'))
ALTER TABLE [dbo].[DNEvents]  WITH NOCHECK ADD  CONSTRAINT [FK_DNEventDetails_DNEvents_DNEventDetailID] FOREIGN KEY([DNEventDetailID])
REFERENCES [dbo].[DNEventDetails] ([DNEventDetailID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_DNEventDetails_DNEvents_DNEventDetailID]') AND parent_object_id = OBJECT_ID(N'[dbo].[DNEvents]'))
ALTER TABLE [dbo].[DNEvents] CHECK CONSTRAINT [FK_DNEventDetails_DNEvents_DNEventDetailID]
GO
